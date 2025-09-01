#pragma once

#include <NimBLEDevice.h>
#include <functional>
#include "XboxBatteryEvent.h"
#include "XboxControlsEvent.h"
#include "utils.h"

template <typename T>
using OnUpdate = std::function<void(T& value)>;

template <typename T>
class BLENotifiableSignal {
 public:
  BLENotifiableSignal(const blegc::BLEValueDecoder<T>& decoder, const blegc::BLECharacteristicLocation& location);
  ~BLENotifiableSignal();
  bool init(NimBLEClient* pClient);
  void readLast(T& out);
  void onUpdate(const OnUpdate<T>& onUpdate);

 private:
  struct Store {
    T event{};
  };
  static void _onUpdateTaskFn(void* pvParameters);
  void _handleNotify(NimBLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify);

  const blegc::BLEValueDecoder<T>& _decoder;
  const blegc::BLECharacteristicLocation& _location;

  NimBLERemoteCharacteristic* _pChar;
  TaskHandle_t _onUpdateTask;
  SemaphoreHandle_t _storeMutex;
  Store _store;
  OnUpdate<T> _onUpdateCallback;
  bool _onUpdateCallbackSet;
};

template class BLENotifiableSignal<XboxControlsEvent>;
template class BLENotifiableSignal<XboxBatteryEvent>;
