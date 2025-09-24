#include "BLEValueWriter.h"

#include <NimBLEDevice.h>
#include <bitset>
#include <functional>
#include "BLECharacteristicSpec.h"
#include "logger.h"
#include "utils.h"
#include "config.h"
#include "xbox/XboxVibrationsCommand.h"

static auto* LOG_TAG = "BLEValueWriter";

constexpr size_t MAX_CAPACITY = CONFIG_BT_BLEGC_WRITER_MAX_CAPACITY;
constexpr size_t INIT_CAPACITY = 8;

template <typename T>
BLEValueWriter<T>::BLEValueWriter(const BLEValueEncoder<T>& encoder,
                                  const BLECharacteristicSpec& charSpec)
    : _encoder(encoder), _charSpec(charSpec), _pChar(nullptr), _sendDataTask(nullptr), _storeMutex(nullptr), _store() {
  _store.capacity = INIT_CAPACITY;
  _store.pBuffer = new uint8_t[_store.capacity];
  _store.pSendBuffer = new uint8_t[_store.capacity];

  _storeMutex = xSemaphoreCreateMutex();
  configASSERT(_storeMutex);
  xTaskCreate(_sendDataFn, "_sendDataFn", 10000, this, 0, &_sendDataTask);
  configASSERT(_sendDataTask);
}
template <typename T>
BLEValueWriter<T>::~BLEValueWriter() {
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
bool BLEValueWriter<T>::init(NimBLEClient* pClient) {
  _pChar = blegc::findCharacteristic(pClient, _charSpec);
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
void BLEValueWriter<T>::write(const T& cmd) {
  configASSERT(xSemaphoreTake(_storeMutex, portMAX_DELAY));
  size_t used;
  BLEEncodeResult result;
  while ((result = _encoder(cmd, used, _store.pBuffer, _store.capacity)) == BLEEncodeResult::BufferTooShort &&
         _store.capacity < MAX_CAPACITY) {
    delete[] _store.pBuffer;
    delete[] _store.pSendBuffer;
    _store.capacity = min(_store.capacity * 2, MAX_CAPACITY);
    _store.pBuffer = new uint8_t[_store.capacity];
    _store.pSendBuffer = new uint8_t[_store.capacity];
  }

  _store.used = result == BLEEncodeResult::Success ? used : 0;
  configASSERT(xSemaphoreGive(_storeMutex));

  switch (result) {
    case BLEEncodeResult::Success:
      xTaskNotifyGive(_sendDataTask);
      break;
    case BLEEncodeResult::InvalidValue:
      BLEGC_LOGE(LOG_TAG, "Encoding failed, invalid value");
      break;
    case BLEEncodeResult::BufferTooShort:
      BLEGC_LOGE(LOG_TAG, "Encoding failed, buffer too short");
      break;
  }
}

template <typename T>
void BLEValueWriter<T>::_sendDataFn(void* pvParameters) {
  auto* self = static_cast<BLEValueWriter*>(pvParameters);

  while (true) {
    ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

    configASSERT(xSemaphoreTake(self->_storeMutex, portMAX_DELAY));

    uint8_t* tmp = self->_store.pSendBuffer;
    self->_store.pSendBuffer = self->_store.pBuffer;
    self->_store.pBuffer = tmp;

    auto used = self->_store.used;
    auto shouldSend = self->_store.used > 0 && self->_store.used <= self->_store.capacity;
    configASSERT(xSemaphoreGive(self->_storeMutex));

    if (!shouldSend) {
      continue;
    }

    if (!self->_pChar) {
      BLEGC_LOGD(LOG_TAG, "Writer not initialized, sending data aborted");
      continue;
    }

    BLEGC_LOGT(LOG_TAG, "Writing value. %s", blegc::remoteCharToStr(self->_pChar).c_str());

    self->_pChar->writeValue(self->_store.pSendBuffer, used);
  }
}

template class BLEValueWriter<XboxVibrationsCommand>;
