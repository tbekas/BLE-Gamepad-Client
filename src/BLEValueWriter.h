#pragma once

#include <NimBLEDevice.h>
#include "utils.h"

template <typename T>
class BLEValueWriter {
 public:
  BLEValueWriter(const blegc::BLEValueEncoder<T>& encoder, const blegc::BLECharacteristicSpec& charSpec);
  ~BLEValueWriter();
  bool init(NimBLEClient* pClient);
  void write(const T& value);

 private:
  struct Store {
    uint8_t* pBuffer{};
    uint8_t* pSendBuffer{};
    size_t used{};
    size_t capacity{};
  };
  static void _sendDataFn(void* pvParameters);

  const blegc::BLEValueEncoder<T>& _encoder;
  const blegc::BLECharacteristicSpec& _charSpec;

  NimBLERemoteCharacteristic* _pChar;
  TaskHandle_t _sendDataTask;
  SemaphoreHandle_t _storeMutex;
  Store _store;
};
