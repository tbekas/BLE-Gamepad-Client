#pragma once

#include <NimBLEDevice.h>
#include "BLEVibrationsCommand.h"
#include "utils.h"

template <typename T>
class BLEOutgoingSignal {
 public:
  using Encoder = std::function<size_t(const T& value, uint8_t buffer[], size_t bufferLen)>;

  BLEOutgoingSignal(const Encoder& encoder, const blegc::CharacteristicFilter& filter, size_t bufferLen = 1);
  ~BLEOutgoingSignal();
  bool init(NimBLEClient* pClient);
  bool deinit(bool disconnected);
  void write(const T& value);

 private:
  struct Store {
    uint8_t* pBuffer{};
    uint8_t* pSendBuffer{};
    size_t used{};
    size_t capacity{};
  };
  static void _sendDataFn(void* pvParameters);

  const Encoder& _encoder;
  const blegc::CharacteristicFilter& _filter;

  NimBLERemoteCharacteristic* _pChar;
  TaskHandle_t _sendDataTask;
  SemaphoreHandle_t _storeMutex;
  Store _store;
};

template class BLEOutgoingSignal<BLEVibrationsCommand>;
