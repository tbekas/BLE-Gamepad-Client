#include "BLEOutgoingSignal.h"

#include <NimBLEDevice.h>
#include <bitset>
#include <functional>
#include "logger.h"
#include "utils.h"

static auto* LOG_TAG = "BLEOutgoingSignal";

constexpr size_t maxCapacity = 1024;

template <typename T>
BLEOutgoingSignal<T>::BLEOutgoingSignal()
    : _initialized(false),
      _encoder([](const T&, uint8_t[], size_t) { return static_cast<size_t>(0); }),
      _address(),
      _pChar(nullptr),
      _sendDataTask(nullptr),
      _storeMutex(nullptr) {}

template <typename T>
bool BLEOutgoingSignal<T>::init(NimBLEAddress address, Spec& spec) {
  if (_initialized) {
    return false;
  }

  _address = address;

  _store.capacity = spec.bufferLen > 0 ? spec.bufferLen : 8;
  _store.pBuffer = new uint8_t[_store.capacity];
  _store.pSendBuffer = new uint8_t[_store.capacity];

  _encoder = spec.encoder;
  _pChar = blegc::findCharacteristic(_address, spec.serviceUUID, spec.characteristicUUID,
                                     [](NimBLERemoteCharacteristic* c) { return c->canWrite(); });
  if (!_pChar) {
    return false;
  }

  _storeMutex = xSemaphoreCreateMutex();
  configASSERT(_storeMutex);
  xTaskCreate(_sendDataFn, "_sendDataFn", 10000, this, 0, &_sendDataTask);
  configASSERT(_sendDataTask);

  _initialized = true;
  return true;
}

template <typename T>
bool BLEOutgoingSignal<T>::deinit(bool disconnected) {
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
bool BLEOutgoingSignal<T>::isInitialized() const {
  return _initialized;
}

template <typename T>
void BLEOutgoingSignal<T>::write(const T& value) {
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
    BLEGC_LOGE(LOG_TAG, "Encoding failed");
    return;
  }

  xTaskNotifyGive(_sendDataTask);
}

template <typename T>
void BLEOutgoingSignal<T>::_sendDataFn(void* pvParameters) {
  auto* self = static_cast<BLEOutgoingSignal*>(pvParameters);

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
      BLEGC_LOGE(LOG_TAG, "Encoding failed");
      continue;
    }

    if (!self->_pChar) {
      BLEGC_LOGE(LOG_TAG, "Remote characteristic not initialized");
      continue;
    }

    BLEGC_LOGT(LOG_TAG, "Writing value. %s", blegc::remoteCharToStr(self->_pChar).c_str());

    self->_pChar->writeValue(self->_store.pSendBuffer, used);
  }
}

template <typename T>
bool BLEOutgoingSignal<T>::Spec::isEnabled() const {
  return !blegc::isNull(serviceUUID);
}

template <typename T>
BLEOutgoingSignal<T>::Spec::operator std::string() const {
  return "service uuid: " + std::string(serviceUUID) + ", characteristic uuid: " + std::string(characteristicUUID);
}
