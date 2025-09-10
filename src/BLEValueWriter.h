#pragma once

#include <NimBLEDevice.h>
#include "BLECharacteristicSpec.h"
#include "coders.h"

template <typename T>
class BLEValueWriter {
 public:
  BLEValueWriter(const BLEValueEncoder<T>& encoder, const BLECharacteristicSpec& charSpec);
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

  const BLEValueEncoder<T>& _encoder;
  const BLECharacteristicSpec& _charSpec;

  NimBLERemoteCharacteristic* _pChar;
  TaskHandle_t _sendDataTask;
  SemaphoreHandle_t _storeMutex;
  Store _store;
};
