#include "BLEOutgoingSignal.h"

#include <NimBLEDevice.h>
#include <bitset>
#include <functional>
#include "logger.h"
#include "utils.h"

static auto* LOG_TAG = "BLEOutgoingSignal";

constexpr size_t maxCapacity = 1024;

template <typename T>
BLEOutgoingSignal<T>::BLEOutgoingSignal(const Encoder& encoder,
                                        const blegc::CharacteristicFilter& filter,
                                        size_t bufferLen)
    : _encoder(encoder), _filter(filter), _pChar(nullptr), _sendDataTask(nullptr), _storeMutex(nullptr), _store() {
  _store.capacity = bufferLen > 0 ? bufferLen : 8;
  _store.pBuffer = new uint8_t[_store.capacity];
  _store.pSendBuffer = new uint8_t[_store.capacity];

  _storeMutex = xSemaphoreCreateMutex();
  configASSERT(_storeMutex);
  xTaskCreate(_sendDataFn, "_sendDataFn", 10000, this, 0, &_sendDataTask);
  configASSERT(_sendDataTask);
}
template <typename T>
BLEOutgoingSignal<T>::~BLEOutgoingSignal() {
  if (_sendDataTask != nullptr) {
    vTaskDelete(_sendDataTask);
  }
  if (_storeMutex != nullptr) {
    vSemaphoreDelete(_storeMutex);
  }

  delete[] _store.pBuffer;
  delete[] _store.pSendBuffer;
}

template <typename T>
bool BLEOutgoingSignal<T>::init(NimBLEClient* pClient) {
  _pChar = blegc::findCharacteristic(pClient, _filter);
  if (!_pChar) {
    return false;
  }

  if (!_pChar->canWrite()) {
    BLEGC_LOGE(LOG_TAG, "Characteristic not able to write. %s", blegc::remoteCharToStr(_pChar).c_str());
    return false;
  }

  return true;
}

template <typename T>
void BLEOutgoingSignal<T>::write(const T& value) {
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
