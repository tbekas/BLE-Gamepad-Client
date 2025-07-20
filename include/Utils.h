#pragma once

#include <NimBLEDevice.h>
#include <string>

class Utils {
 public:
  static std::string remoteCharToStr(NimBLERemoteCharacteristic* pChar) {
    auto str = pChar->toString();

    std::string::size_type pos = str.find('\n');
    if (pos != std::string::npos) {
      return str.substr(0, pos);
    }
    return str;
  }

  static bool discoverAttributes(const NimBLEAddress address) {
    auto pClient = BLEDevice::getClientByPeerAddress(address);
    if (!pClient) {
      BLEGC_LOGE("BLE client not found, address %s", std::string(address).c_str());
      return false;
    }
    if (!pClient->discoverAttributes()) {
      BLEGC_LOGE("Failed to discover attributes, address %s", std::string(address).c_str());
      return false;
    }

    #if CONFIG_BLEGC_LOG_LEVEL >= 4
    for (auto& pService : pClient->getServices(false)) {
      BLEGC_LOGD("Discovered service %s", std::string(pService->getUUID()).c_str());
      for (auto& pChar   : pService->getCharacteristics(false)) {
        BLEGC_LOGD("Discovered characteristic %s", Utils::remoteCharToStr(pChar).c_str());
      }
    }
    #endif
    return true;
  }

  using BLECharacteristicFilter = std::function<bool(NimBLERemoteCharacteristic*)>;

  static NimBLERemoteCharacteristic* findCharacteristic(
      const NimBLEAddress address,
      const NimBLEUUID serviceUUID,
      const NimBLEUUID characteristicUUID = NimBLEUUID(),
      const BLECharacteristicFilter& filter = [](NimBLERemoteCharacteristic*) { return true; }) {
    BLEGC_LOGD("Looking up for characteristic. Service uuid: %s, characteristic uuid %s.",
               std::string(serviceUUID).c_str(), std::string(characteristicUUID).c_str());

    auto pBleClient = NimBLEDevice::getClientByPeerAddress(address);
    if (!pBleClient) {
      BLEGC_LOGE("BLE client not found, address %s", std::string(address).c_str());
      return nullptr;
    }

    auto pService = pBleClient->getService(serviceUUID);
    if (!pService) {
      BLEGC_LOGE("Service not found, serviceUUID: %s", std::string(serviceUUID).c_str());
      return nullptr;
    }

    for (auto pChar : pService->getCharacteristics(false)) {
      if (!std::string(characteristicUUID).empty() && characteristicUUID != pChar->getUUID()) {
        BLEGC_LOGD("Characteristic uuid different from lookup uuid. %s", Utils::remoteCharToStr(pChar).c_str());
        continue;
      }

      if (!filter(pChar)) {
        BLEGC_LOGD("Characteristic doesn't pass filter. %s", Utils::remoteCharToStr(pChar).c_str());
        continue;
      }

      return pChar;
    }

    return nullptr;
  }
};
