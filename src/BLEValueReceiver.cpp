#include "BLEValueReceiver.h"

#include <NimBLEDevice.h>
#include <bitset>
#include <functional>
#include "logger.h"
#include "steam/SteamControlsState.h"
#include "utils.h"
#include "xbox/XboxBatteryState.h"
#include "xbox/XboxControlsState.h"

template <typename T>
BLEValueReceiver<T>::BLEValueReceiver()
    :
      _callbackTask(nullptr),
      _storeMutex(nullptr),
      _store(),
      _onValueChangedCallback(),
      _onValueChangedCallbackSet(false) {
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

  _store = Store();

  if (!pChar->canNotify()) {
    BLEGC_LOGE("Characteristic not able to notify. %s", blegc::remoteCharToStr(pChar).c_str());
    return false;
  }

  auto handlerFn = std::bind(&BLEValueReceiver::_handleNotify, this, std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3, std::placeholders::_4);

  BLEGC_LOGD("Subscribing to notifications. %s", blegc::remoteCharToStr(pChar).c_str());

  if (!pChar->subscribe(true, handlerFn, false)) {
    BLEGC_LOGE("Failed to subscribe to notifications. %s", blegc::remoteCharToStr(pChar).c_str());
    return false;
  }

  BLEGC_LOGD("Successfully subscribed to notifications. %s", blegc::remoteCharToStr(pChar).c_str());

  auto* pClient = pChar->getClient();
  if (pClient) {
    _store.value.controllerAddress = pClient->getPeerAddress();
  }
  return true;
}

template <typename T>
void BLEValueReceiver<T>::read(T* value) {
  configASSERT(xSemaphoreTake(_storeMutex, portMAX_DELAY));
  *value = _store.value;
  configASSERT(xSemaphoreGive(_storeMutex));
}

template <typename T>
void BLEValueReceiver<T>::onValueChanged(const OnValueChanged<T>& callback) {
  _onValueChangedCallback = callback;
  _onValueChangedCallbackSet = true;
}

template <typename T>
void BLEValueReceiver<T>::_callbackTaskFn(void* pvParameters) {
  auto* self = static_cast<BLEValueReceiver*>(pvParameters);

  while (true) {
    ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

    configASSERT(xSemaphoreTake(self->_storeMutex, portMAX_DELAY));
    auto valueCopy = self->_store.value;
    configASSERT(xSemaphoreGive(self->_storeMutex));
    self->_onValueChangedCallback(valueCopy);
  }
}

template <typename T>
void BLEValueReceiver<T>::_handleNotify(NimBLERemoteCharacteristic* pChar,
                                        uint8_t* pData,
                                        size_t dataLen,
                                        bool isNotify) {
  BLEGC_LOGV("Received a notification. %s", blegc::remoteCharToStr(pChar).c_str());

  configASSERT(xSemaphoreTake(_storeMutex, portMAX_DELAY));
  BLEDecodeResult result;
  bool runCallback;
  if (_onValueChangedCallbackSet) {
    auto valueCopy = _store.value;
    result = _store.value.decode(pData, dataLen);
    runCallback = valueCopy != _store.value;
  } else {
    result = _store.value.decode(pData, dataLen);
    runCallback = false;
  }

#if CONFIG_BT_BLEGC_LOG_BUFFER_ENABLED
  if (result == BLEDecodeResult::Success) {
    if (_store.value.reportDataCap < dataLen) {
      _store.value.reportData = std::make_shared<uint8_t[]>(dataLen);
      _store.value.reportDataCap = dataLen;
    }

    _store.value.reportDataLen = dataLen;
    memcpy(_store.value.reportData.get(), pData, dataLen);
  }
#endif
  configASSERT(xSemaphoreGive(_storeMutex));

  switch (result) {
    case BLEDecodeResult::Success:
      if (runCallback) {
        xTaskNotifyGive(_callbackTask);
      }
      break;
    case BLEDecodeResult::NotSupported:
      BLEGC_LOGV("Report not supported. %s", blegc::remoteCharToStr(pChar).c_str());
      break;
    case BLEDecodeResult::InvalidReport:
      BLEGC_LOGE("Invalid report. %s", blegc::remoteCharToStr(pChar).c_str());
      break;
  }
}

template class BLEValueReceiver<XboxControlsState>;
template class BLEValueReceiver<XboxBatteryState>;
template class BLEValueReceiver<SteamControlsState>;
