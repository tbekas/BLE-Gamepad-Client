#pragma once

#include <NimBLEDevice.h>
#include <functional>
#include "BLECharacteristicSpec.h"
#include "coders.h"

template <typename T>
using OnUpdate = std::function<void(T& value)>;

template <typename T>
class BLEValueReceiver {
 public:
  BLEValueReceiver(const BLEValueDecoder<T>& decoder, const BLECharacteristicSpec& charSpec);
  ~BLEValueReceiver();
  bool init(NimBLEClient* pClient);
  void readLast(T& out);
  void onUpdate(const OnUpdate<T>& onUpdate);

 private:
  struct Store {
    T event{};
  };
  static void _callbackTaskFn(void* pvParameters);
  void _handleNotify(NimBLERemoteCharacteristic* pChar, uint8_t* pData, size_t dataLen, bool isNotify);

  const BLEValueDecoder<T>& _decoder;
  const BLECharacteristicSpec& _charSpec;

  NimBLERemoteCharacteristic* _pChar;
  TaskHandle_t _callbackTask;
  SemaphoreHandle_t _storeMutex;
  Store _store;
  OnUpdate<T> _onUpdateCallback;
  bool _onUpdateCallbackSet;
};
