#pragma once

#include <NimBLEDevice.h>
#include "BLEOutgoingSignalAdapter.h"
#include "BLEVibrationsCommand.h"

template <typename T>
class BLEOutgoingSignal {
 public:
  BLEOutgoingSignal();
  ~BLEOutgoingSignal() = default;
  bool init(NimBLEAddress address, BLEOutgoingSignalAdapter<T>& adapter);
  bool deinit(bool disconnected);
  bool isInitialized() const;
  void write(const T& value);

 private:
  struct Store {
    uint8_t* pBuffer{};
    uint8_t* pSendBuffer{};
    size_t used{};
    size_t capacity{};
  };
  static void _sendDataFn(void* pvParameters);
  bool _initialized;
  BLESignalEncoder<T> _encoder;
  NimBLEAddress _address;
  NimBLERemoteCharacteristic* _pChar;
  TaskHandle_t _sendDataTask;
  SemaphoreHandle_t _storeMutex;
  Store _store;
};

template class BLEOutgoingSignal<BLEVibrationsCommand>;

using BLEVibrationsSignal = BLEOutgoingSignal<BLEVibrationsCommand>;
