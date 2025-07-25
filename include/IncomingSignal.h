#pragma once

#include <NimBLEDevice.h>
#include <functional>
#include "BatteryEvent.h"
#include "ControlsEvent.h"
#include "IncomingSignalConfig.h"

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
  bool init(NimBLEAddress address, IncomingSignalConfig<T>& config);
  bool deinit(bool disconnected);
  bool isInitialized() const;
  void read(T& out);
  void onUpdate(const OnUpdate<T>& onUpdate);

 private:
  static void _callConsumerFn(void* pvParameters);
  void _handleNotify(NimBLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify);
  bool _initialized;
  OnUpdate<T> _onUpdate;
  bool _onUpdateSet;
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
