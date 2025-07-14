#pragma once

#include <NimBLEUUID.h>
#include <string>
#include "Parser.h"

template <typename T>
struct SignalConfig {
  SignalConfig() : parser(), serviceUUID(), characteristicUUID() {}
  Parser<T> parser;
  NimBLEUUID serviceUUID;
  NimBLEUUID characteristicUUID;

  bool isEnabled() const {
    return std::string(serviceUUID).size() > 0;
  }

  bool isDisabled() const {
    return !isEnabled();
  }

  operator std::string() const;
};

template <typename T>
SignalConfig<T>::operator std::string() const {
  return "serviceUUID: " + std::string(serviceUUID) + ", characteristicUUID: " + std::string(characteristicUUID);
}