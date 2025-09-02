#pragma once

#include <NimBLEDevice.h>
#include "utils.h"

template <typename T>
class BLEWritableSignal {
 public:
  BLEWritableSignal(const blegc::BLEValueEncoder<T>& encoder, const blegc::BLECharacteristicLocation& location);
  ~BLEWritableSignal();
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
  const blegc::BLECharacteristicLocation& _location;

  NimBLERemoteCharacteristic* _pChar;
  TaskHandle_t _sendDataTask;
  SemaphoreHandle_t _storeMutex;
  Store _store;
};
