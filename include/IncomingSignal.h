#pragma once

#include <NimBLEDevice.h>
#include <functional>
#include "Logger.h"
#include "SignalCoder.h"
#include "SignalConfig.h"
#include "ControlsEvent.h"
#include "BatteryEvent.h"

template <typename T>
using Consumer = std::function<void(T& value)>;

template <typename T>
class IncomingSignal {
 public:
  struct Store {
    T event;
    bool updated;
  };

  explicit IncomingSignal(NimBLEAddress address);
  ~IncomingSignal() = default;

  void read(T& out);
  bool isUpdated() const;
  bool isInitialized() const;
  void subscribe(const Consumer<T>& onUpdate);

  bool init(IncomingSignalConfig<T>& config);
  bool deinit(bool disconnected);

 private:
  static void _callConsumerFn(void* pvParameters);
  bool _initialized;
  Consumer<T> _consumer;
  bool _hasSubscription;
  void _handleNotify(NimBLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify);
  SignalDecoder<T> _decoder;
  NimBLEAddress _address;
  NimBLERemoteCharacteristic* _pChar;
  TaskHandle_t _callConsumerTask;
  SemaphoreHandle_t _storeMutex;
  Store _store;
};

template class IncomingSignal<ControlsEvent>;
template class IncomingSignal<BatteryEvent>;

using ControlsSignal = IncomingSignal<ControlsEvent>;
using BatterySignal = IncomingSignal<BatteryEvent>;
