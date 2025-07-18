#pragma once

#include <NimBLEUUID.h>
#include <string>
#include "SignalCoder.h"

struct SignalConfig {
  NimBLEUUID serviceUUID;
  NimBLEUUID characteristicUUID;

  bool isEnabled() const { return !std::string(serviceUUID).empty(); }

  bool isDisabled() const { return !isEnabled(); }

  explicit operator std::string() const;
};

inline SignalConfig::operator std::string() const {
  return "serviceUUID: " + std::string(serviceUUID) + ", characteristicUUID: " + std::string(characteristicUUID);
}

template <typename T>
struct IncomingSignalConfig : SignalConfig {
  SignalDecoder<T> decoder;
};

template <typename T>
struct OutgoingSignalConfig : SignalConfig {
  SignalEncoder<T> encoder;

  /// @brief Optional. Specifies the size of the buffer for the encoded payload. Leave undefined if the encoded size
  /// varies depending on the input.
  size_t bufferLen{};
};
