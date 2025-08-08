#include "BLEIncomingSignal.h"

#include <NimBLEDevice.h>
#include <bitset>
#include <functional>
#include "logger.h"
#include "utils.h"

static auto* LOG_TAG = "BLEIncomingSignal";

template <typename T>
BLEIncomingSignal<T>::BLEIncomingSignal()
    : _initialized(false),
      _onUpdate([](T&) {}),
      _onUpdateSet(false),
      _decoder([](T&, uint8_t[], size_t) { return 1; }),
      _address(),
      _pChar(nullptr),
      _onUpdateTask(nullptr),
      _storeMutex(nullptr),
      _store({.event = T()}) {}

template <typename T>
bool BLEIncomingSignal<T>::init(NimBLEAddress address, Adapter& adapter) {
  if (_initialized) {
    return false;
  }
  _address = address;

  _storeMutex = xSemaphoreCreateMutex();
  configASSERT(_storeMutex);
  xTaskCreate(_onUpdateTaskFn, "_onUpdateTask", 10000, this, 0, &_onUpdateTask);
  configASSERT(_onUpdateTask);

  _decoder = adapter.decoder;
  _pChar = blegc::findCharacteristic(_address, adapter.serviceUUID, adapter.characteristicUUID,
                                     [](NimBLERemoteCharacteristic* c) { return c->canNotify(); });
  if (!_pChar) {
    return false;
  }

  auto handlerFn = std::bind(&BLEIncomingSignal<T>::_handleNotify, this, std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3, std::placeholders::_4);

  BLEGC_LOGD(LOG_TAG, "Subscribing to notifications. %s", blegc::remoteCharToStr(_pChar).c_str());

  if (!_pChar->subscribe(true, handlerFn, true)) {
    BLEGC_LOGE(LOG_TAG, "Failed to subscribe to notifications. %s", blegc::remoteCharToStr(_pChar).c_str());
    return false;
  }

  BLEGC_LOGD(LOG_TAG, "Successfully subscribed to notifications. %s", blegc::remoteCharToStr(_pChar).c_str());

  _initialized = true;
  return true;
}

template <typename T>
bool BLEIncomingSignal<T>::deinit(bool disconnected) {
  if (!_initialized) {
    return false;
  }

  bool result = true;
  if (!disconnected) {
    if (_pChar) {
      if (!_pChar->unsubscribe()) {
        BLEGC_LOGW(LOG_TAG, "Failed to unsubscribe from notifications. %s", blegc::remoteCharToStr(_pChar).c_str());
        result = false;
      } else {
        BLEGC_LOGD(LOG_TAG, "Successfully unsubscribed from notifications. %s", blegc::remoteCharToStr(_pChar).c_str());
      }
    }
  }

  if (_onUpdateTask != nullptr) {
    vTaskDelete(_onUpdateTask);
  }
  if (_storeMutex != nullptr) {
    vSemaphoreDelete(_storeMutex);
  }

  _pChar = nullptr;

  _initialized = false;
  return result;
}

template <typename T>
bool BLEIncomingSignal<T>::isInitialized() const {
  return _initialized;
}

template <typename T>
void BLEIncomingSignal<T>::read(T& out) {
  if (!_initialized) {
    return;
  }

  configASSERT(xSemaphoreTake(_storeMutex, portMAX_DELAY));
  out = _store.event;
  configASSERT(xSemaphoreGive(_storeMutex));
}

template <typename T>
void BLEIncomingSignal<T>::onUpdate(const OnUpdate<T>& onUpdate) {
  _onUpdate = onUpdate;
  _onUpdateSet = true;
}

template <typename T>
void BLEIncomingSignal<T>::_onUpdateTaskFn(void* pvParameters) {
  auto* self = static_cast<BLEIncomingSignal*>(pvParameters);

  while (true) {
    ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

    configASSERT(xSemaphoreTake(self->_storeMutex, portMAX_DELAY));
    auto eventCopy = self->_store.event;
    configASSERT(xSemaphoreGive(self->_storeMutex));
    self->_onUpdate(eventCopy);
  }
}

template <typename T>
void BLEIncomingSignal<T>::_handleNotify(NimBLERemoteCharacteristic* pChar,
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

  if (_onUpdateSet && result) {
    xTaskNotifyGive(_onUpdateTask);
  }
}

template <typename T>
bool BLEIncomingSignal<T>::Adapter::isEnabled() const {
  return !blegc::isNull(serviceUUID);
}
template <typename T>
BLEIncomingSignal<T>::Adapter::operator std::string() const {
  return "service uuid: " + std::string(serviceUUID) + ", characteristic uuid: " + std::string(characteristicUUID);
}
