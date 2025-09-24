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
BLEValueReceiver<T>::BLEValueReceiver()
    :
      _callbackTask(nullptr),
      _storeMutex(nullptr),
      _store(),
      _onUpdateCallback(),
      _onUpdateCallbackSet(false) {
  _storeMutex = xSemaphoreCreateMutex();
  configASSERT(_storeMutex);
  xTaskCreate(_callbackTaskFn, "_callbackTask", 10000, this, 0, &_callbackTask);
  configASSERT(_callbackTask);
}

template <typename T>
BLEValueReceiver<T>::~BLEValueReceiver() {
  if (_callbackTask != nullptr) {
    vTaskDelete(_callbackTask);
    _callbackTask = nullptr;
  }
  if (_storeMutex != nullptr) {
    vSemaphoreDelete(_storeMutex);
    _storeMutex = nullptr;
  }
}

template <typename T>
bool BLEValueReceiver<T>::init(NimBLERemoteCharacteristic* pChar) {
  if (!pChar) {
    return false;
  }

  if (!pChar->canNotify()) {
    BLEGC_LOGE(LOG_TAG, "Characteristic not able to notify. %s", blegc::remoteCharToStr(pChar).c_str());
    return false;
  }

  auto handlerFn = std::bind(&BLEValueReceiver::_handleNotify, this, std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3, std::placeholders::_4);

  BLEGC_LOGD(LOG_TAG, "Subscribing to notifications. %s", blegc::remoteCharToStr(pChar).c_str());

  if (!pChar->subscribe(true, handlerFn, false)) {
    BLEGC_LOGE(LOG_TAG, "Failed to subscribe to notifications. %s", blegc::remoteCharToStr(pChar).c_str());
    return false;
  }

  BLEGC_LOGD(LOG_TAG, "Successfully subscribed to notifications. %s", blegc::remoteCharToStr(pChar).c_str());

  auto* pClient = pChar->getClient();
  if (pClient) {
    _store.event.controllerAddress = pClient->getPeerAddress();
  }
  return true;
}

template <typename T>
void BLEValueReceiver<T>::read(T& event) {
  configASSERT(xSemaphoreTake(_storeMutex, portMAX_DELAY));
  event = _store.event;
  configASSERT(xSemaphoreGive(_storeMutex));
}

template <typename T>
void BLEValueReceiver<T>::onUpdate(const OnUpdate<T>& callback) {
  _onUpdateCallback = callback;
  _onUpdateCallbackSet = true;
}

template <typename T>
void BLEValueReceiver<T>::_callbackTaskFn(void* pvParameters) {
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
#if CONFIG_BT_BLEGC_COPY_REPORT_DATA
    if (_store.event.dataCap < dataLen) {
      _store.event.data = std::make_shared<uint8_t[]>(dataLen);
      _store.event.dataCap = dataLen;
      _store.event.dataLen = dataLen;
    }

    memcpy(_store.event.data.get(), pData, dataLen);
#endif

  auto result = _store.event.decode(pData, dataLen);
  configASSERT(xSemaphoreGive(_storeMutex));

  switch (result) {
    case BLEDecodeResult::Success:
      if (_onUpdateCallbackSet) {
        xTaskNotifyGive(_callbackTask);
      }
      break;
    case BLEDecodeResult::NotSupported:
      BLEGC_LOGT(LOG_TAG, "Report not supported. %s", blegc::remoteCharToStr(pChar).c_str());
      break;
    case BLEDecodeResult::InvalidReport:
      BLEGC_LOGE(LOG_TAG, "Invalid report. %s", blegc::remoteCharToStr(pChar).c_str());
      break;
  }
}

template class BLEValueReceiver<XboxControlsEvent>;
template class BLEValueReceiver<XboxBatteryEvent>;
template class BLEValueReceiver<SteamControlsEvent>;
