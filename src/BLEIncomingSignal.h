#pragma once

#include <NimBLEDevice.h>
#include <functional>
#include "BLEBatteryEvent.h"
#include "BLEControlsEvent.h"
#include "BLEIncomingSignalAdapter.h"

template <typename T>
using OnUpdate = std::function<void(T& value)>;

template <typename T>
class BLEIncomingSignal {
 public:
  BLEIncomingSignal();
  ~BLEIncomingSignal() = default;
  bool init(NimBLEAddress address, BLEIncomingSignalAdapter<T>& adapter);
  bool deinit(bool disconnected);
  bool isInitialized() const;
  void read(T& out);
  void onUpdate(const OnUpdate<T>& onUpdate);

 private:
  struct Store {
    T event;
  };
  static void _onUpdateTaskFn(void* pvParameters);
  void _handleNotify(NimBLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify);
  bool _initialized;
  OnUpdate<T> _onUpdate;
  bool _onUpdateSet;
  BLESignalDecoder<T> _decoder;
  NimBLEAddress _address;
  NimBLERemoteCharacteristic* _pChar;
  TaskHandle_t _onUpdateTask;
  SemaphoreHandle_t _storeMutex;
  Store _store;
};

template class BLEIncomingSignal<BLEControlsEvent>;
template class BLEIncomingSignal<BLEBatteryEvent>;

using BLEControlsSignal = BLEIncomingSignal<BLEControlsEvent>;
using BLEBatterySignal = BLEIncomingSignal<BLEBatteryEvent>;
