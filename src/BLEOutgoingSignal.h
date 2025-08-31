#pragma once

#include <NimBLEDevice.h>
#include "BLEVibrationsCommand.h"

template <typename T>
class BLEOutgoingSignal {
 public:
  using Encoder = std::function<size_t(const T& value, uint8_t buffer[], size_t bufferLen)>;

  struct Spec {
    bool enabled{false};
    NimBLEUUID serviceUUID{};
    NimBLEUUID characteristicUUID{};
    unsigned int idx{0};
    Encoder encoder{};

    /// @brief Optional. Specifies the size of the buffer for the encoded payload. Leave undefined if the encoded size
    /// varies depending on the input.
    size_t bufferLen{};
    explicit operator std::string() const;
  };

  BLEOutgoingSignal();
  ~BLEOutgoingSignal() = default;
  bool init(NimBLEClient* pClient, const Spec& spec);
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
  Encoder _encoder;
  NimBLEAddress _address;
  NimBLERemoteCharacteristic* _pChar;
  TaskHandle_t _sendDataTask;
  SemaphoreHandle_t _storeMutex;
  Store _store;
};

template class BLEOutgoingSignal<BLEVibrationsCommand>;

using BLEVibrationsSignal = BLEOutgoingSignal<BLEVibrationsCommand>;
