#pragma once

#include <NimBLEDevice.h>
#include <string>
#include "logger.h"

namespace utils {

  static bool isNull(const NimBLEUUID& uuid) { return uuid.bitSize() == 0; }

  static std::string remoteCharToStr(const NimBLERemoteCharacteristic* pChar) {
    auto str = pChar->toString();

    std::string::size_type pos = str.find('\n');
    if (pos != std::string::npos) {
      return str.substr(0, pos);
    }
    return str;
  }

  static bool discoverAttributes(const NimBLEAddress address) {
    auto* pClient = BLEDevice::getClientByPeerAddress(address);
    if (!pClient) {
      BLEGC_LOGE("BLE client not found, address %s", std::string(address).c_str());
      return false;
    }
    if (!pClient->discoverAttributes()) {
      BLEGC_LOGE("Failed to discover attributes, address %s", std::string(address).c_str());
      return false;
    }

#if CONFIG_BT_BLEGC_LOG_LEVEL >= 4
    for (auto& pService : pClient->getServices(false)) {
      BLEGC_LOGD("Discovered service %s", std::string(pService->getUUID()).c_str());
      for (auto& pChar : pService->getCharacteristics(false)) {
        BLEGC_LOGD("Discovered characteristic %s", remoteCharToStr(pChar).c_str());
      }
    }
#endif
    return true;
  }

  using BLECharacteristicFilter = std::function<bool(NimBLERemoteCharacteristic*)>;

  static NimBLERemoteCharacteristic* findCharacteristic(
      const NimBLEAddress address,
      const NimBLEUUID& serviceUUID,
      const NimBLEUUID& characteristicUUID = NimBLEUUID(),
      const BLECharacteristicFilter& filter = [](NimBLERemoteCharacteristic*) { return true; }) {
    BLEGC_LOGD("Looking up for characteristic, service uuid: %s, characteristic uuid: %s.",
               std::string(serviceUUID).c_str(),
               isNull(characteristicUUID) ? "null" : std::string(characteristicUUID).c_str());

    auto* pBleClient = NimBLEDevice::getClientByPeerAddress(address);
    if (!pBleClient) {
      BLEGC_LOGE("BLE client not found, address %s", std::string(address).c_str());
      return nullptr;
    }

    auto* pService = pBleClient->getService(serviceUUID);
    if (!pService) {
      BLEGC_LOGE("Service not found, service uuid: %s", std::string(serviceUUID).c_str());
      return nullptr;
    }

    for (auto* pChar : pService->getCharacteristics(false)) {
      if (!isNull(characteristicUUID) && characteristicUUID != pChar->getUUID()) {
        continue;
      }

      if (!filter(pChar)) {
        continue;
      }

      return pChar;
    }

    BLEGC_LOGE("Characteristic not found, service uuid: %s, characteristic uuid: %s.", std::string(serviceUUID).c_str(),
               isNull(characteristicUUID) ? "null" : std::string(characteristicUUID).c_str());

    return nullptr;
  }
};
