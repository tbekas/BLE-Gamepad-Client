#pragma once

#include <NimBLEDevice.h>
#include <functional>
#include "logger.h"
#include "SignalCoder.h"
#include "SignalConfig.h"
#include "ControlsEvent.h"
#include "BatteryEvent.h"

template <typename T>
using OnUpdate = std::function<void(T& value)>;

template <typename T>
class IncomingSignal {
 public:
  struct Store {
    T event;
  };

  IncomingSignal();
  ~IncomingSignal() = default;

  void read(T& out);
  bool isInitialized() const;
  void onUpdate(const OnUpdate<T>& onUpdate);

  bool init(NimBLEAddress address, IncomingSignalConfig<T>& config);
  bool deinit(bool disconnected);

 private:
  static void _callConsumerFn(void* pvParameters);
  bool _initialized;
  OnUpdate<T> _onUpdate;
  bool _onUpdateSet;
  void _handleNotify(NimBLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify);
  SignalDecoder<T> _decoder;
  NimBLEAddress _address;
  NimBLERemoteCharacteristic* _pChar;
  TaskHandle_t _callOnUpdateTask;
  SemaphoreHandle_t _storeMutex;
  Store _store;
};

template class IncomingSignal<ControlsEvent>;
template class IncomingSignal<BatteryEvent>;

using ControlsSignal = IncomingSignal<ControlsEvent>;
using BatterySignal = IncomingSignal<BatteryEvent>;
