#pragma once

#include <NimBLEDevice.h>
#include <NimBLERemoteCharacteristic.h>
#include <bitset>
#include <functional>
#include "Logger.h"
#include "SignalCoder.h"
#include "SignalConfig.h"
#include "Utils.h"

#define FLAG_COUNT 2
#define FLAG_UPDATED_IDX 0
#define FLAG_DECODED_IDX 1

template <typename T>
using Consumer = std::function<void(T& value)>;
template <typename T>
class InputSignal {
 public:
  struct Store {
    T event;
    uint8_t* pBuffer{};
    size_t used{};
    size_t capacity{};
    std::bitset<FLAG_COUNT> flags;
  };

  explicit InputSignal(NimBLEAddress address);
  ~InputSignal() = default;

  T& read();
  bool isUpdated() const;
  bool isInitialized() const;
  void subscribe(const Consumer<T>& onUpdate);

  bool init(SignalConfig<T>& config);
  bool deinit(bool disconnected);

 private:
  static void _callConsumerFn(void* pvParameters);
  bool _initialized;
  Consumer<T> _consumer;
  bool _hasSubscription;
  void _handleNotify(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
  SignalDecoder<T> _decoder;
  NimBLEAddress _address;
  NimBLEUUID _serviceUUID;
  NimBLEUUID _characteristicUUID;
  TaskHandle_t _callConsumerTask;
  SemaphoreHandle_t _storeMutex;
  Store _store;
};

template <typename T>
InputSignal<T>::InputSignal(const NimBLEAddress address)
    : _initialized(false),
      _consumer([](T&) {}),
      _hasSubscription(false),
      _decoder([](T&, uint8_t[], size_t) { return true; }),
      _address(address),
      _callConsumerTask(nullptr),
      _storeMutex(nullptr),
	  _store() {
  _store.flags[FLAG_DECODED_IDX] = true;
}

template <typename T>
bool InputSignal<T>::init(SignalConfig<T>& config) {
  if (_initialized) {
    return false;
  }

  _storeMutex = xSemaphoreCreateMutex();
  configASSERT(_storeMutex);
  configASSERT(xSemaphoreGive(_storeMutex));
  xTaskCreate(_callConsumerFn, "_bleConnectionMsgConsumerTask", 10000, this, 0, &_callConsumerTask);
  configASSERT(_callConsumerTask);

  _decoder = config.decoder;
  _store.pBuffer = new uint8_t[config.payloadLen];
  _store.capacity = config.payloadLen;
  auto pChar = Utils::findCharacteristic(_address, config.serviceUUID, config.characteristicUUID,
                                         [](NimBLERemoteCharacteristic* c) { return c->canNotify(); });
  if (!pChar) {
    return false;
  }
  _serviceUUID = pChar->getRemoteService()->getUUID();
  _characteristicUUID = pChar->getUUID();

  auto handlerFn = std::bind(&InputSignal<T>::_handleNotify, this, std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3, std::placeholders::_4);

  BLEGC_LOGD("Subscribing to notifications. %s", Utils::remoteCharToStr(pChar).c_str());

  if (!pChar->subscribe(true, handlerFn, true)) {
    BLEGC_LOGE("Failed to subscribe to notifications. %s", Utils::remoteCharToStr(pChar).c_str());
    return false;
  }

  BLEGC_LOGD("Successfully subscribed to notifications. %s", Utils::remoteCharToStr(pChar).c_str());

  _initialized = true;
  return true;
}

template <typename T>
bool InputSignal<T>::deinit(bool disconnected) {
  if (!_initialized) {
    return false;
  }

  bool result = true;
  if (!disconnected) {
    auto pChar = Utils::findCharacteristic(_address, _serviceUUID, _characteristicUUID);
    if (pChar) {
      if (!pChar->unsubscribe()) {
        BLEGC_LOGW("Failed to unsubscribe from notifications. %s", Utils::remoteCharToStr(pChar).c_str());
        result = false;
      } else {
        BLEGC_LOGD("Successfully unsubscribed from notifications. %s", Utils::remoteCharToStr(pChar).c_str());
      }
    }
  }

  delete[] _store.pBuffer;

  if (_callConsumerTask != nullptr) {
    vTaskDelete(_callConsumerTask);
  }
  if (_storeMutex != nullptr) {
    vSemaphoreDelete(_storeMutex);
  }

  _initialized = false;
  return result;
}

template <typename T>
void InputSignal<T>::_callConsumerFn(void* pvParameters) {
  auto* self = (InputSignal<T>*)pvParameters;

  while (true) {
    ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
    self->_consumer(self->_store.event);
  }
}

template <typename T>
void InputSignal<T>::_handleNotify(NimBLERemoteCharacteristic* pRemoteCharacteristic,
                                      uint8_t* pData,
                                      size_t length,
                                      bool isNotify) {
  BLEGC_LOGD("Received a notification. %s", Utils::remoteCharToStr(pRemoteCharacteristic).c_str());

  if (_store.capacity < length) {
    delete[] _store.pBuffer;
    _store.pBuffer = new uint8_t[length];
    _store.capacity = length;
  }

  configASSERT(xSemaphoreTake(_storeMutex, portMAX_DELAY));
  std::memcpy(_store.pBuffer, pData, length);
  _store.used = length;

  if (_hasSubscription) {
    auto result = _decoder(_store.event, _store.pBuffer, _store.used);
    _store.event.controllerAddress = _address;
    _store.flags[FLAG_DECODED_IDX] = result;
    _store.flags[FLAG_UPDATED_IDX] = result;
    if (result) {
      xTaskNotifyGive(_callConsumerTask);
    }
  } else {
    _store.flags[FLAG_DECODED_IDX] = false;
    _store.flags[FLAG_UPDATED_IDX] = true;
  }
  configASSERT(xSemaphoreGive(_storeMutex));
}

template <typename T>
T& InputSignal<T>::read() {
  configASSERT(xSemaphoreTake(_storeMutex, portMAX_DELAY));
  if (!_store.flags[FLAG_DECODED_IDX]) {
    auto result = _decoder(_store.event, _store.pBuffer, _store.used);
    _store.event.controllerAddress = _address;
    _store.flags[FLAG_DECODED_IDX] = result;
  }

  if (_store.flags[FLAG_UPDATED_IDX]) {
    _store.flags[FLAG_UPDATED_IDX] = false;
  }

  T& val = _store.event;
  configASSERT(xSemaphoreGive(_storeMutex));
  return val;
}

template <typename T>
bool InputSignal<T>::isUpdated() const {
  return _store.flags[FLAG_UPDATED_IDX];
}

template <typename T>
void InputSignal<T>::subscribe(const Consumer<T>& onUpdate) {
  _consumer = onUpdate;
  _hasSubscription = true;
}

template <typename T>
bool InputSignal<T>::isInitialized() const {
  return _initialized;
}
