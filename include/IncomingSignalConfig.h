#pragma once

#include <NimBLEUUID.h>
#include <functional>
#include <string>
#include "Utils.h"

template <typename T>
using SignalDecoder = std::function<size_t(T&, uint8_t payload[], size_t payloadLen)>;

template <typename T>
struct IncomingSignalConfig {
  NimBLEUUID serviceUUID{};
  NimBLEUUID characteristicUUID{};
  SignalDecoder<T> decoder{};

  bool isEnabled() const { return !Utils::isNull(serviceUUID); }
  bool isDisabled() const { return !isEnabled(); }
  explicit operator std::string() const;
};

template <typename T>
IncomingSignalConfig<T>::operator std::string() const {
  return "service uuid: " + std::string(serviceUUID) + ", characteristic uuid: " + std::string(characteristicUUID);
}
