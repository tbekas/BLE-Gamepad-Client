#pragma once

#include <NimBLEUUID.h>
#include <functional>
#include <string>
#include "BLEHelpers.h"

template <typename T>
using BLESignalDecoder = std::function<size_t(T&, uint8_t payload[], size_t payloadLen)>;

template <typename T>
struct BLEIncomingSignalAdapter {
  NimBLEUUID serviceUUID{};
  NimBLEUUID characteristicUUID{};
  BLESignalDecoder<T> decoder{};

  bool isEnabled() const { return !BLEHelpers::isNull(serviceUUID); }
  bool isDisabled() const { return !isEnabled(); }
  explicit operator std::string() const;
};

template <typename T>
BLEIncomingSignalAdapter<T>::operator std::string() const {
  return "service uuid: " + std::string(serviceUUID) + ", characteristic uuid: " + std::string(characteristicUUID);
}
