#include "BLENotifiableSignal.h"

#include <NimBLEDevice.h>
#include <bitset>
#include <functional>
#include "logger.h"
#include "utils.h"

static auto* LOG_TAG = "BLEIncomingSignal";

template <typename T>
BLENotifiableSignal<T>::BLENotifiableSignal(const blegc::BLEValueDecoder<T>& decoder,
                                            const blegc::BLECharacteristicLocation& location)
    : _decoder(decoder),
      _location(location),
      _pChar(nullptr),
      _onUpdateTask(nullptr),
      _storeMutex(nullptr),
      _store(),
      _onUpdateCallback(),
      _onUpdateCallbackSet(false) {
  _storeMutex = xSemaphoreCreateMutex();
  configASSERT(_storeMutex);
  xTaskCreate(_onUpdateTaskFn, "_onUpdateTask", 10000, this, 0, &_onUpdateTask);
  configASSERT(_onUpdateTask);
}

template <typename T>
BLENotifiableSignal<T>::~BLENotifiableSignal() {
  if (_onUpdateTask != nullptr) {
    vTaskDelete(_onUpdateTask);
    _onUpdateTask = nullptr;
  }
  if (_storeMutex != nullptr) {
    vSemaphoreDelete(_storeMutex);
    _storeMutex = nullptr;
  }
}

template <typename T>
bool BLENotifiableSignal<T>::init(NimBLEClient* pClient) {
  _pChar = blegc::findCharacteristic(pClient, _location);
  if (!_pChar) {
    return false;
  }

  if (!_pChar->canNotify()) {
    BLEGC_LOGE(LOG_TAG, "Characteristic not able to notify. %s", blegc::remoteCharToStr(_pChar).c_str());
    return false;
  }

  auto handlerFn = std::bind(&BLENotifiableSignal::_handleNotify, this, std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3, std::placeholders::_4);

  BLEGC_LOGD(LOG_TAG, "Subscribing to notifications. %s", blegc::remoteCharToStr(_pChar).c_str());

  if (!_pChar->subscribe(true, handlerFn, false)) {
    BLEGC_LOGE(LOG_TAG, "Failed to subscribe to notifications. %s", blegc::remoteCharToStr(_pChar).c_str());
    return false;
  }

  BLEGC_LOGD(LOG_TAG, "Successfully subscribed to notifications. %s", blegc::remoteCharToStr(_pChar).c_str());

  return true;
}

template <typename T>
void BLENotifiableSignal<T>::readLast(T& out) {
  configASSERT(xSemaphoreTake(_storeMutex, portMAX_DELAY));
  out = _store.event;
  configASSERT(xSemaphoreGive(_storeMutex));
}

template <typename T>
void BLENotifiableSignal<T>::onUpdate(const OnUpdate<T>& onUpdate) {
  _onUpdateCallback = onUpdate;
  _onUpdateCallbackSet = true;
}

template <typename T>
void BLENotifiableSignal<T>::_onUpdateTaskFn(void* pvParameters) {
  auto* self = static_cast<BLENotifiableSignal*>(pvParameters);

  while (true) {
    ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

    configASSERT(xSemaphoreTake(self->_storeMutex, portMAX_DELAY));
    auto eventCopy = self->_store.event;
    configASSERT(xSemaphoreGive(self->_storeMutex));
    self->_onUpdateCallback(eventCopy);
  }
}

template <typename T>
void BLENotifiableSignal<T>::_handleNotify(NimBLERemoteCharacteristic* pChar,
                                           uint8_t* pData,
                                           size_t length,
                                           bool isNotify) {
  BLEGC_LOGT(LOG_TAG, "Received a notification. %s", blegc::remoteCharToStr(pChar).c_str());

  configASSERT(xSemaphoreTake(_storeMutex, portMAX_DELAY));
  auto result = _decoder(_store.event, pData, length) > 0;
  configASSERT(xSemaphoreGive(_storeMutex));

  if (!result) {
    BLEGC_LOGE(LOG_TAG, "Decoding failed. %s", blegc::remoteCharToStr(pChar).c_str());
  }

  if (_onUpdateCallbackSet && result) {
    xTaskNotifyGive(_onUpdateTask);
  }
}
