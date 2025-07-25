#pragma once

#include <NimBLEUUID.h>
#include <string>
#include "SignalCoder.h"
#include "Utils.h"

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

template <typename T>
struct OutgoingSignalConfig {
  NimBLEUUID serviceUUID{};
  NimBLEUUID characteristicUUID{};
  SignalEncoder<T> encoder{};

  /// @brief Optional. Specifies the size of the buffer for the encoded payload. Leave undefined if the encoded size
  /// varies depending on the input.
  size_t bufferLen{};

  bool isEnabled() const { return !Utils::isNull(serviceUUID); }
  bool isDisabled() const { return !isEnabled(); }
  explicit operator std::string() const;
};

template <typename T>
OutgoingSignalConfig<T>::operator std::string() const {
  return "service uuid: " + std::string(serviceUUID) + ", characteristic uuid: " + std::string(characteristicUUID);
}
