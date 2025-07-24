#pragma once

#include <NimBLEDevice.h>
#include "SignalCoder.h"
#include "SignalConfig.h"
#include "VibrationsCommand.h"

template <typename T>
class OutgoingSignal {
 public:
  struct Store {
    uint8_t* pBuffer{};
    uint8_t* pSendBuffer{};
    size_t used{};
    size_t capacity{};
  };

  OutgoingSignal();
  ~OutgoingSignal() = default;

  void write(const T& value);
  bool isInitialized() const;

  bool init(NimBLEAddress address, OutgoingSignalConfig<T>& config);
  bool deinit(bool disconnected);

 private:
  static void _sendDataFn(void* pvParameters);
  bool _initialized;
  SignalEncoder<T> _encoder;
  NimBLEAddress _address;
  NimBLERemoteCharacteristic* _pChar;
  TaskHandle_t _sendDataTask;
  SemaphoreHandle_t _storeMutex;
  Store _store;
};

template class OutgoingSignal<VibrationsCommand>;

using VibrationsSignal = OutgoingSignal<VibrationsCommand>;
