#include "OutgoingSignal.h"
#include <NimBLEDevice.h>
#include <NimBLERemoteCharacteristic.h>
#include <bitset>
#include <functional>
#include "logger.h"
#include "SignalCoder.h"
#include "SignalConfig.h"
#include "Utils.h"

constexpr size_t maxCapacity = 1024;

template <typename T>
OutgoingSignal<T>::OutgoingSignal()
    : _initialized(false),
      _encoder([](const T&, uint8_t[], size_t) { return static_cast<size_t>(0); }),
      _address(),
      _pChar(nullptr),
      _sendDataTask(nullptr),
      _storeMutex(nullptr) {}

template <typename T>
void OutgoingSignal<T>::_sendDataFn(void* pvParameters) {
  auto* self = static_cast<OutgoingSignal*>(pvParameters);

  while (true) {
    ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

    configASSERT(xSemaphoreTake(self->_storeMutex, portMAX_DELAY));

    uint8_t* tmp = self->_store.pSendBuffer;
    self->_store.pSendBuffer = self->_store.pBuffer;
    self->_store.pBuffer = tmp;

    auto used = self->_store.used;
    auto result = self->_store.used > 0 && self->_store.used <= self->_store.capacity;
    configASSERT(xSemaphoreGive(self->_storeMutex));

    if (!result) {
      BLEGC_LOGE("Encoding failed");
      continue;
    }

    if (!self->_pChar) {
      BLEGC_LOGE("Remote characteristic not initialized");
      continue;
    }

    BLEGC_LOGT("Writing value. %s", Utils::remoteCharToStr(_pChar));

    self->_pChar->writeValue(self->_store.pSendBuffer, used);
  }
}

template <typename T>
void OutgoingSignal<T>::write(const T& value) {
  if (!_initialized) {
    return;
  }

  configASSERT(xSemaphoreTake(_storeMutex, portMAX_DELAY));
  size_t used;
  while ((used = _encoder(value, _store.pBuffer, _store.capacity)) == 0 && _store.capacity < maxCapacity) {
    delete[] _store.pBuffer;
    delete[] _store.pSendBuffer;
    _store.capacity = min(_store.capacity * 2, maxCapacity);
    _store.pBuffer = new uint8_t[_store.capacity];
    _store.pSendBuffer = new uint8_t[_store.capacity];
  }

  _store.used = used;
  auto result = _store.used > 0 && _store.used <= _store.capacity;
  configASSERT(xSemaphoreGive(_storeMutex));

  if (!result) {
    BLEGC_LOGE("Encoding failed");
    return;
  }

  xTaskNotifyGive(_sendDataTask);
}

template <typename T>
bool OutgoingSignal<T>::init(NimBLEAddress address, OutgoingSignalConfig<T>& config) {
  if (_initialized) {
    return false;
  }

  _address = address;

  _store.capacity = config.bufferLen > 0 ? config.bufferLen : 8;
  _store.pBuffer = new uint8_t[_store.capacity];
  _store.pSendBuffer = new uint8_t[_store.capacity];

  _encoder = config.encoder;
  _pChar = Utils::findCharacteristic(_address, config.serviceUUID, config.characteristicUUID,
                                     [](NimBLERemoteCharacteristic* c) { return c->canWrite(); });
  if (!_pChar) {
    return false;
  }

  _storeMutex = xSemaphoreCreateMutex();
  configASSERT(_storeMutex);
  configASSERT(xSemaphoreGive(_storeMutex));
  xTaskCreate(_sendDataFn, "_sendDataFn", 10000, this, 0, &_sendDataTask);
  configASSERT(_sendDataTask);

  _initialized = true;
  return true;
}

template <typename T>
bool OutgoingSignal<T>::deinit(bool disconnected) {
  if (!_initialized) {
    return false;
  }

  if (_sendDataTask != nullptr) {
    vTaskDelete(_sendDataTask);
  }
  if (_storeMutex != nullptr) {
    vSemaphoreDelete(_storeMutex);
  }

  _pChar = nullptr;

  delete[] _store.pBuffer;
  delete[] _store.pSendBuffer;

  _initialized = false;
  return true;
}

template <typename T>
bool OutgoingSignal<T>::isInitialized() const {
  return _initialized;
}
