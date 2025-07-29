#include "BLEIncomingSignal.h"

#include <NimBLEDevice.h>
#include <NimBLERemoteCharacteristic.h>
#include <bitset>
#include <functional>
#include "BLEIncomingSignalAdapter.h"
#include "BLEHelpers.h"
#include "logger.h"

template <typename T>
BLEIncomingSignal<T>::BLEIncomingSignal()
    : _initialized(false),
      _onUpdate([](T&) {}),
      _onUpdateSet(false),
      _decoder([](T&, uint8_t[], size_t) { return 1; }),
      _address(),
      _pChar(nullptr),
      _callOnUpdateTask(nullptr),
      _storeMutex(nullptr),
      _store({.event = T()}) {}

template <typename T>
bool BLEIncomingSignal<T>::init(NimBLEAddress address, BLEIncomingSignalAdapter<T>& config) {
  if (_initialized) {
    return false;
  }
  _address = address;

  _storeMutex = xSemaphoreCreateMutex();
  configASSERT(_storeMutex);
  configASSERT(xSemaphoreGive(_storeMutex));
  xTaskCreate(_callConsumerFn, "_callConsumerFn", 10000, this, 0, &_callOnUpdateTask);
  configASSERT(_callOnUpdateTask);

  _decoder = config.decoder;
  _pChar = BLEHelpers::findCharacteristic(_address, config.serviceUUID, config.characteristicUUID,
                                     [](NimBLERemoteCharacteristic* c) { return c->canNotify(); });
  if (!_pChar) {
    return false;
  }

  auto handlerFn = std::bind(&BLEIncomingSignal<T>::_handleNotify, this, std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3, std::placeholders::_4);

  BLEGC_LOGD("Subscribing to notifications. %s", BLEHelpers::remoteCharToStr(_pChar).c_str());

  if (!_pChar->subscribe(true, handlerFn, true)) {
    BLEGC_LOGE("Failed to subscribe to notifications. %s", BLEHelpers::remoteCharToStr(_pChar).c_str());
    return false;
  }

  BLEGC_LOGD("Successfully subscribed to notifications. %s", BLEHelpers::remoteCharToStr(_pChar).c_str());

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
        BLEGC_LOGW("Failed to unsubscribe from notifications. %s", BLEHelpers::remoteCharToStr(_pChar).c_str());
        result = false;
      } else {
        BLEGC_LOGD("Successfully unsubscribed from notifications. %s", BLEHelpers::remoteCharToStr(_pChar).c_str());
      }
    }
  }

  if (_callOnUpdateTask != nullptr) {
    vTaskDelete(_callOnUpdateTask);
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
void BLEIncomingSignal<T>::_callConsumerFn(void* pvParameters) {
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
void BLEIncomingSignal<T>::_handleNotify(NimBLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
  BLEGC_LOGT("Received a notification. %s", Utils::remoteCharToStr(pChar).c_str());

  configASSERT(xSemaphoreTake(_storeMutex, portMAX_DELAY));
  auto result = _decoder(_store.event, pData, length) > 0;
  configASSERT(xSemaphoreGive(_storeMutex));

  if (!result) {
    BLEGC_LOGE("Decoding failed. %s", BLEHelpers::remoteCharToStr(pChar).c_str());
  }

  if (_onUpdateSet && result) {
    xTaskNotifyGive(_callOnUpdateTask);
  }
}
