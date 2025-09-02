#include "BLEWritableSignal.h"

#include <NimBLEDevice.h>
#include <bitset>
#include <functional>
#include "xbox/XboxVibrationsCommand.h"
#include "logger.h"
#include "utils.h"

static auto* LOG_TAG = "BLEWritableSignal";

constexpr size_t MAX_CAPACITY = 1024;
constexpr size_t INIT_CAPACITY = 8;

template <typename T>
BLEWritableSignal<T>::BLEWritableSignal(const blegc::BLEValueEncoder<T>& encoder,
                                        const blegc::BLECharacteristicLocation& location)
    : _encoder(encoder), _location(location), _pChar(nullptr), _sendDataTask(nullptr), _storeMutex(nullptr), _store() {
  _store.capacity = INIT_CAPACITY;
  _store.pBuffer = new uint8_t[_store.capacity];
  _store.pSendBuffer = new uint8_t[_store.capacity];

  _storeMutex = xSemaphoreCreateMutex();
  configASSERT(_storeMutex);
  xTaskCreate(_sendDataFn, "_sendDataFn", 10000, this, 0, &_sendDataTask);
  configASSERT(_sendDataTask);
}
template <typename T>
BLEWritableSignal<T>::~BLEWritableSignal() {
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
bool BLEWritableSignal<T>::init(NimBLEClient* pClient) {
  _pChar = blegc::findCharacteristic(pClient, _location);
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
void BLEWritableSignal<T>::write(const T& value) {
  configASSERT(xSemaphoreTake(_storeMutex, portMAX_DELAY));
  size_t used;
  while ((used = _encoder(value, _store.pBuffer, _store.capacity)) == 0 && _store.capacity < MAX_CAPACITY) {
    delete[] _store.pBuffer;
    delete[] _store.pSendBuffer;
    _store.capacity = min(_store.capacity * 2, MAX_CAPACITY);
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
void BLEWritableSignal<T>::_sendDataFn(void* pvParameters) {
  auto* self = static_cast<BLEWritableSignal*>(pvParameters);

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

template class BLEWritableSignal<XboxVibrationsCommand>;
