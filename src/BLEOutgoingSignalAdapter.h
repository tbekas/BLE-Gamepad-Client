#pragma once

#include <NimBLEUUID.h>
#include <functional>
#include <string>
#include "utils.h"

template <typename T>
using BLESignalEncoder = std::function<size_t(const T& value, uint8_t buffer[], size_t bufferLen)>;

template <typename T>
struct BLEOutgoingSignalAdapter {
  NimBLEUUID serviceUUID{};
  NimBLEUUID characteristicUUID{};
  BLESignalEncoder<T> encoder{};

  /// @brief Optional. Specifies the size of the buffer for the encoded payload. Leave undefined if the encoded size
  /// varies depending on the input.
  size_t bufferLen{};

  bool isEnabled() const { return !blegc::isNull(serviceUUID); }
  bool isDisabled() const { return !isEnabled(); }
  explicit operator std::string() const;
};

template <typename T>
BLEOutgoingSignalAdapter<T>::operator std::string() const {
  return "service uuid: " + std::string(serviceUUID) + ", characteristic uuid: " + std::string(characteristicUUID);
}
