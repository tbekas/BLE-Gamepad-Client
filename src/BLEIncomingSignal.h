#pragma once

#include <NimBLEDevice.h>
#include <functional>
#include "BLEBatteryEvent.h"
#include "BLEControlsEvent.h"
#include "utils.h"

template <typename T>
using OnUpdate = std::function<void(T& value)>;

template <typename T>
class BLEIncomingSignal {
 public:
  using Decoder = std::function<size_t(T&, uint8_t payload[], size_t payloadLen)>;

  BLEIncomingSignal(const Decoder& decoder, const blegc::CharacteristicFilter& filter);
  ~BLEIncomingSignal();
  bool init(NimBLEClient* pClient);
  void read(T& out);
  void onUpdate(const OnUpdate<T>& onUpdate);

 private:
  struct Store {
    T event{};
  };
  static void _onUpdateTaskFn(void* pvParameters);
  void _handleNotify(NimBLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify);

  const Decoder& _decoder;
  const blegc::CharacteristicFilter& _filter;

  NimBLERemoteCharacteristic* _pChar;
  TaskHandle_t _onUpdateTask;
  SemaphoreHandle_t _storeMutex;
  Store _store;
  OnUpdate<T> _onUpdateCallback;
  bool _onUpdateCallbackSet;
};

template class BLEIncomingSignal<BLEControlsEvent>;
template class BLEIncomingSignal<BLEBatteryEvent>;
