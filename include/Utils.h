#pragma once

#include <NimBLERemoteCharacteristic.h>
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

  using BLECharacteristicFilter = std::function<bool(NimBLERemoteCharacteristic*)>;

  static NimBLERemoteCharacteristic* findCharacteristic(
      const NimBLEAddress address,
      const NimBLEUUID serviceUUID,
      const NimBLEUUID characteristicUUID = NimBLEUUID(),
      const BLECharacteristicFilter& filter = [](NimBLERemoteCharacteristic*) { return true; }) {

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

    if (!std::string(characteristicUUID).empty()) {
      auto pChar = pService->getCharacteristic(characteristicUUID);
      if (!pChar) {
        BLEGC_LOGE("Characteristic not found, characteristicUUID: %s", std::string(characteristicUUID).c_str());
        return nullptr;
      }

      if (!filter(pChar)) {
        BLEGC_LOGE("Characteristic found, but it doesn't meet criteria. %s", Utils::remoteCharToStr(pChar).c_str());
        return nullptr;
      }

      return pChar;
    }

    BLEGC_LOGD("Characteristics in a service %s", std::string(serviceUUID).c_str());

    for (auto pChar : pService->getCharacteristics(true)) {
      BLEGC_LOGD("A characteristic: %s", Utils::remoteCharToStr(pChar).c_str());
    }

    // lookup any characteristic that can notify
    for (auto pChar : pService->getCharacteristics(false)) {
      if (!filter(pChar)) {
        BLEGC_LOGD("Skipping characteristic that doesn't meet criteria. %s", Utils::remoteCharToStr(pChar).c_str());
        continue;
      }
      BLEGC_LOGD("Found characteristic that meet criteria. %s", Utils::remoteCharToStr(pChar).c_str());
      return pChar;
    }

    BLEGC_LOGE("Unable to find any characteristic that can notify");

    return nullptr;
  }
};
