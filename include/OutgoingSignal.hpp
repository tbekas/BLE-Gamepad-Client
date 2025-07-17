#pragma once

#include <NimBLEDevice.h>
#include <NimBLERemoteCharacteristic.h>
#include <bitset>
#include <functional>
#include "Logger.h"
#include "SignalCoder.h"
#include "SignalConfig.h"
#include "Utils.h"

template <typename T>
class OutgoingSignal {
 public:
  struct Store {
    uint8_t* pBuffer{};
    size_t used{};
    size_t capacity{};
  };
  explicit OutgoingSignal(NimBLEAddress address);
  ~OutgoingSignal() = default;

  bool isInitialized() const;

  bool init(SignalConfig<T>& config);
  bool deinit(bool disconnected);

 private:
  bool _initialized;

  SignalEncoder<T> _encoder;
  NimBLEAddress _address;
  NimBLEUUID _serviceUUID;
  NimBLEUUID _characteristicUUID;
  TaskHandle_t _writeReportTask;
  SemaphoreHandle_t _storeMutex;
};

template <typename T>
OutgoingSignal<T>::OutgoingSignal(NimBLEAddress address) :
	_initialized(false),
	_encoder([](const T&, uint8_t[], size_t&) { return true; }),
	_address(address),
	_writeReportTask(nullptr),
	_storeMutex(nullptr)
{}

template <typename T>
bool OutgoingSignal<T>::init(SignalConfig<T>& config) {
  if (_initialized) {
    return false;
  }

  _storeMutex = xSemaphoreCreateMutex();
  configASSERT(_storeMutex);
  configASSERT(xSemaphoreGive(_storeMutex));
  xTaskCreate(_writeReportFn, "_writeReportFn", 10000, this, 0, &_writeReportTask);
  configASSERT(_writeReportTask);

  _initialized = true;
  return true;
}

template <typename T>
bool OutgoingSignal<T>::deinit(bool disconnected) {
  if (!_initialized) {
    return false;
  }

  _initialized = false;
  return result;
}

template <typename T>
bool OutgoingSignal<T>::isInitialized() const {
  return _initialized;
}
