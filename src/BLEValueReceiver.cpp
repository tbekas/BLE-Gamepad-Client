#include "BLEValueReceiver.h"

#include <NimBLEDevice.h>
#include <bitset>
#include <functional>
#include "logger.h"
#include "steam/SteamControlsEvent.h"
#include "utils.h"
#include "xbox/XboxBatteryEvent.h"
#include "xbox/XboxControlsEvent.h"

static auto* LOG_TAG = "BLEValueReceiver";

template <typename T>
BLEValueReceiver<T>::BLEValueReceiver(const blegc::BLEValueDecoder<T>& decoder,
                                      const blegc::BLECharacteristicSpec& charSpec)
    : _decoder(decoder),
      _charSpec(charSpec),
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
BLEValueReceiver<T>::~BLEValueReceiver() {
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
bool BLEValueReceiver<T>::init(NimBLEClient* pClient) {
  _pChar = blegc::findCharacteristic(pClient, _charSpec);
  if (!_pChar) {
    return false;
  }

  if (!_pChar->canNotify()) {
    BLEGC_LOGE(LOG_TAG, "Characteristic not able to notify. %s", blegc::remoteCharToStr(_pChar).c_str());
    return false;
  }

  auto handlerFn = std::bind(&BLEValueReceiver::_handleNotify, this, std::placeholders::_1, std::placeholders::_2,
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
void BLEValueReceiver<T>::readLast(T& out) {
  configASSERT(xSemaphoreTake(_storeMutex, portMAX_DELAY));
  out = _store.event;
  configASSERT(xSemaphoreGive(_storeMutex));
}

template <typename T>
void BLEValueReceiver<T>::onUpdate(const OnUpdate<T>& onUpdate) {
  _onUpdateCallback = onUpdate;
  _onUpdateCallbackSet = true;
}

template <typename T>
void BLEValueReceiver<T>::_onUpdateTaskFn(void* pvParameters) {
  auto* self = static_cast<BLEValueReceiver*>(pvParameters);

  while (true) {
    ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

    configASSERT(xSemaphoreTake(self->_storeMutex, portMAX_DELAY));
    auto eventCopy = self->_store.event;
    configASSERT(xSemaphoreGive(self->_storeMutex));
    self->_onUpdateCallback(eventCopy);
  }
}

template <typename T>
void BLEValueReceiver<T>::_handleNotify(NimBLERemoteCharacteristic* pChar,
                                        uint8_t* pData,
                                        size_t dataLen,
                                        bool isNotify) {
  BLEGC_LOGT(LOG_TAG, "Received a notification. %s", blegc::remoteCharToStr(pChar).c_str());

  configASSERT(xSemaphoreTake(_storeMutex, portMAX_DELAY));
  if (std::is_base_of<BLEBaseEvent, T>::value) {
    BLEBaseEvent& e = _store.event;

    if (auto* pClient = pChar->getClient()) {
      e.controllerAddress = pClient->getPeerAddress();
    }

#if CONFIG_BT_BLEGC_COPY_REPORT_DATA
    if (e.dataCap < dataLen) {
      e.data = std::make_shared<uint8_t[]>(dataLen);
      e.dataCap = dataLen;
      e.dataLen = dataLen;
    }

    memcpy(e.data.get(), pData, dataLen);
#endif
  }

  auto result = _decoder(_store.event, pData, dataLen);
  configASSERT(xSemaphoreGive(_storeMutex));

  switch (result) {
    case blegc::BLEDecodeResult::Success:
      if (_onUpdateCallbackSet) {
        xTaskNotifyGive(_onUpdateTask);
      }
      break;
    case blegc::BLEDecodeResult::NotSupported:
      BLEGC_LOGT(LOG_TAG, "Report not supported. %s", blegc::remoteCharToStr(pChar).c_str());
      break;
    case blegc::BLEDecodeResult::InvalidReport:
      BLEGC_LOGE(LOG_TAG, "Invalid report. %s", blegc::remoteCharToStr(pChar).c_str());
      break;
  }
}

template class BLEValueReceiver<XboxControlsEvent>;
template class BLEValueReceiver<XboxBatteryEvent>;
template class BLEValueReceiver<SteamControlsEvent>;
