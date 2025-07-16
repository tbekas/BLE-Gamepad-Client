#pragma once

#include <NimBLEUUID.h>
#include <string>
#include "SignalCoder.h"

template <typename T>
struct SignalConfig {
  SignalConfig() : decoder(), payloadLen(0) {}
  SignalDecoder<T> decoder;
  size_t payloadLen;
  NimBLEUUID serviceUUID;
  NimBLEUUID characteristicUUID;

  bool isEnabled() const { return !std::string(serviceUUID).empty(); }

  bool isDisabled() const { return !isEnabled(); }

  explicit operator std::string() const;
};

template <typename T>
SignalConfig<T>::operator std::string() const {
  return "serviceUUID: " + std::string(serviceUUID) + ", characteristicUUID: " + std::string(characteristicUUID) +
         ", payloadLen: " + std::to_string(payloadLen);
}