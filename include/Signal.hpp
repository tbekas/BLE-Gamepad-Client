#pragma once

#include <NimBLEDevice.h>
#include <NimBLERemoteCharacteristic.h>
#include <NimBLERemoteService.h>
#include <bitset>
#include <functional>
#include "Logger.h"
#include "Parser.h"
#include "SignalConfig.h"
#include "Utils.h"

#define FLAG_COUNT 2
#define FLAG_UPDATED_IDX 0
#define FLAG_PARSED_IDX 1

template <typename T>
using Consumer = std::function<void(T& value)>;
template <typename T>
class Signal {
 public:
  struct Store {
    T event;
    uint8_t* data;
    size_t length;
    std::bitset<FLAG_COUNT> flags;
  };

  Signal(NimBLEAddress address);
  ~Signal() = default;

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
  Parser<T> _parser;
  NimBLEAddress _address;
  NimBLEUUID _serviceUUID;
  NimBLEUUID _characteristicUUID;
  NimBLERemoteCharacteristic* _findCharacteristic(SignalConfig<T>& config);
  NimBLERemoteCharacteristic* _getCharacteristic();

  TaskHandle_t _callConsumerTask;
  SemaphoreHandle_t _storeMutex;
  Store _store;
};

template <typename T>
Signal<T>::Signal(NimBLEAddress address) : _address(address) {
  _initialized = false;
  _parser = [](uint8_t data[], size_t len) { return T(); };
  _consumer = [](T val) {};
  _hasSubscription = false;
  _store = {T(), new uint8_t[0], 0, std::bitset<FLAG_COUNT>()};
  _store.flags[FLAG_PARSED_IDX] = true;
}

template <typename T>
bool Signal<T>::init(SignalConfig<T>& config) {
  if (_initialized) {
    return false;
  }

  _storeMutex = xSemaphoreCreateMutex();
  configASSERT(_storeMutex);
  configASSERT(xSemaphoreGive(_storeMutex));
  xTaskCreate(_callConsumerFn, "_bleConnectionMsgConsumerTask", 10000, this, 0, &_callConsumerTask);
  configASSERT(_callConsumerTask);

  _parser = config.parser;
  auto pChar = _findCharacteristic(config);
  if (!pChar) {
    return false;
  }
  _serviceUUID = pChar->getRemoteService()->getUUID();
  _characteristicUUID = pChar->getUUID();

  auto handlerFn = std::bind(&Signal<T>::_handleNotify, this, std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3, std::placeholders::_4);

  BLEGC_LOGD("Subscribing to notifications. %s", Utils::remoteCharToStr(pChar).c_str());

  if (!pChar->subscribe(true, handlerFn, false)) {
    BLEGC_LOGE("Failed to subscribe to notifications. %s", Utils::remoteCharToStr(pChar).c_str());
    return false;
  }

  BLEGC_LOGD("Successfully subscribed to notifications. %s", Utils::remoteCharToStr(pChar).c_str());

  _initialized = true;
  return true;
}

template <typename T>
NimBLERemoteCharacteristic* Signal<T>::_findCharacteristic(SignalConfig<T>& config) {
  BLEGC_LOGD("Finding a characteristic, config: %s", std::string(config).c_str());

  auto pBleClient = NimBLEDevice::getClientByPeerAddress(_address);
  if (!pBleClient) {
    BLEGC_LOGE("BLE client not found, address %s", std::string(_address).c_str());
    return nullptr;
  }

  auto pService = pBleClient->getService(config.serviceUUID);
  if (!pService) {
    BLEGC_LOGE("Service not found, serviceUUID: %s", std::string(config.serviceUUID).c_str());
    return nullptr;
  }

  if (!std::string(config.characteristicUUID).empty()) {
    auto pChar = pService->getCharacteristic(config.characteristicUUID);
    if (!pChar) {
      BLEGC_LOGE("Characteristic not found, characteristicUUID: %s", std::string(config.characteristicUUID).c_str());
      return nullptr;
    }

    if (!pChar->canNotify()) {
      BLEGC_LOGE("Characteristic found, but it cant't notify. %s", Utils::remoteCharToStr(pChar).c_str());
      return nullptr;
    }

    return pChar;
  }

  BLEGC_LOGD("Looking for any characteristic that can notify");

  // lookup any characteristic that can notify
  for (auto pChar : pService->getCharacteristics(true)) {
    if (!pChar->canNotify()) {
      BLEGC_LOGD("Skipping characteristic that can't notify. %s", Utils::remoteCharToStr(pChar).c_str());
      continue;
    }
    BLEGC_LOGD("Found characteristic that can notify. %s", Utils::remoteCharToStr(pChar).c_str());
    return pChar;
  }

  BLEGC_LOGE("Unable to find any characteristic that can notify");

  return nullptr;
}

template <typename T>
NimBLERemoteCharacteristic* Signal<T>::_getCharacteristic() {
  auto pClient = BLEDevice::getClientByPeerAddress(_address);
  if (!pClient) {
    BLEGC_LOGE("BLE client not found, address: %s", std::string(_address).c_str());
    return nullptr;
  }

  auto pService = pClient->getService(_serviceUUID);
  if (!pService) {
    BLEGC_LOGE("Service not found, serviceUUID: %s", std::string(_serviceUUID).c_str());
    return nullptr;
  }

  auto pChar = pService->getCharacteristic(_characteristicUUID);
  if (!pChar) {
    BLEGC_LOGE("Characteristic not found, characteristicUUID: %s", std::string(_characteristicUUID).c_str());
    return nullptr;
  }

  return pChar;
}

template <typename T>
bool Signal<T>::deinit(bool disconnected) {
  if (!_initialized) {
    return false;
  }

  bool result = true;
  if (!disconnected) {
    auto pChar = _getCharacteristic();
    if (pChar) {
      if (!pChar->unsubscribe()) {
        BLEGC_LOGW("Failed to unsubscribe from notifications. %s", Utils::remoteCharToStr(pChar).c_str());
        result = false;
      } else {
        BLEGC_LOGD("Successfully unsubscribed from notifications. %s", Utils::remoteCharToStr(pChar).c_str());
      }
    }
  }

  if (_callConsumerTask != NULL) {
    vTaskDelete(_callConsumerTask);
  }
  if (_storeMutex != NULL) {
    vSemaphoreDelete(_storeMutex);
  }

  _initialized = false;
  return result;
}

template <typename T>
void Signal<T>::_callConsumerFn(void* pvParameters) {
  Signal<T>* self = (Signal<T>*)pvParameters;

  while (true) {
    ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
    self->_consumer(self->_store.event);
  }
}

template <typename T>
void Signal<T>::_handleNotify(NimBLERemoteCharacteristic* pRemoteCharacteristic,
                              uint8_t* pData,
                              size_t length,
                              bool isNotify) {
  BLEGC_LOGD("Received a notification. %s", Utils::remoteCharToStr(pRemoteCharacteristic).c_str());

  configASSERT(xSemaphoreTake(_storeMutex, portMAX_DELAY));
  if (_store.length != length) {
    delete[] _store.data;
    _store.length = length;
    _store.data = new uint8_t[_store.length];
  }

  std::memcpy(_store.data, pData, _store.length);

  if (_hasSubscription) {
    _store.event = _parser(_store.data, _store.length);
    _store.event.controllerAddress = _address;
    _store.flags[FLAG_PARSED_IDX] = true;
    _store.flags[FLAG_UPDATED_IDX] = true;
    xTaskNotifyGive(_callConsumerTask);
  } else {
    _store.flags[FLAG_PARSED_IDX] = false;
    _store.flags[FLAG_UPDATED_IDX] = true;
  }
  configASSERT(xSemaphoreGive(_storeMutex));
}

template <typename T>
T& Signal<T>::read() {
  configASSERT(xSemaphoreTake(_storeMutex, portMAX_DELAY));
  if (!_store.flags[FLAG_PARSED_IDX]) {
    _store.event = _parser(_store.data, _store.length);
    _store.event.controllerAddress = _address;
    _store.flags[FLAG_PARSED_IDX] = true;
  }

  if (_store.flags[FLAG_UPDATED_IDX]) {
    _store.flags[FLAG_UPDATED_IDX] = false;
  }

  T& val = _store.event;
  configASSERT(xSemaphoreGive(_storeMutex));
  return val;
}

template <typename T>
bool Signal<T>::isUpdated() const {
  return _store.flags[FLAG_UPDATED_IDX];
}

template <typename T>
void Signal<T>::subscribe(const Consumer<T>& onUpdate) {
  _consumer = onUpdate;
  _hasSubscription = true;
}

template <typename T>
bool Signal<T>::isInitialized() const {
  return _initialized;
}