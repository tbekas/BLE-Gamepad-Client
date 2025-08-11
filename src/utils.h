#pragma once

#include <NimBLEDevice.h>
#include <string>
#include "logger.h"

namespace blegc {

static auto* LOG_TAG = "utils";

static bool isNull(const NimBLEUUID& uuid) {
  return uuid.bitSize() == 0;
}

static std::string remoteSvcToStr(const NimBLERemoteService* pSvc) {
  std::string res = "Service: uuid: " + std::string(pSvc->getUUID());
  res += ", handle: ";
  res += std::to_string(pSvc->getHandle());
  return res;
}

static std::string remoteCharToStr(const NimBLERemoteCharacteristic* pChar) {
  std::string res = "Characteristic: uuid: " + std::string(pChar->getUUID());
  res += ", handle: ";
  res += std::to_string(pChar->getHandle());
  res += ", can: [";
  if (pChar->canBroadcast()) {
    res += "broadcast, ";
  }
  if (pChar->canRead()) {
    res += "read, ";
  }
  if (pChar->canWriteNoResponse()) {
    res += "writeNoResponse, ";
  }
  if (pChar->canWrite()) {
    res += "write, ";
  }
  if (pChar->canNotify()) {
    res += "notify, ";
  }
  if (pChar->canIndicate()) {
    res += "indicate, ";
  }
  if (pChar->canWriteSigned()) {
    res += "writeSigned, ";
  }
  if (pChar->hasExtendedProps()) {
    res += "haveExtendedProperties, ";
  }
  auto pos  = res.rfind(", ");
  if (pos == res.size() - 2) {
    res.erase(pos);
  }
  res += "]";

  return res;
}

static bool discoverAttributes(const NimBLEAddress address) {
  auto* pClient = BLEDevice::getClientByPeerAddress(address);
  if (!pClient) {
    BLEGC_LOGE(LOG_TAG, "BLE client not found, address %s", std::string(address).c_str());
    return false;
  }
  if (!pClient->discoverAttributes()) {
    BLEGC_LOGE(LOG_TAG, "Failed to discover attributes, address %s", std::string(address).c_str());
    return false;
  }

#if CONFIG_BT_BLEGC_LOG_LEVEL >= 4
  for (auto& pService : pClient->getServices(false)) {
    BLEGC_LOGD(LOG_TAG, "Discovered %s", remoteSvcToStr(pService).c_str());
    for (auto& pChar : pService->getCharacteristics(false)) {
      BLEGC_LOGD(LOG_TAG, "Discovered %s", remoteCharToStr(pChar).c_str());
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
  BLEGC_LOGD(LOG_TAG, "Looking up for characteristic, service uuid: %s, characteristic uuid: %s.",
             std::string(serviceUUID).c_str(),
             isNull(characteristicUUID) ? "null" : std::string(characteristicUUID).c_str());

  auto* pBleClient = NimBLEDevice::getClientByPeerAddress(address);
  if (!pBleClient) {
    BLEGC_LOGE(LOG_TAG, "BLE client not found, address %s", std::string(address).c_str());
    return nullptr;
  }

  auto* pService = pBleClient->getService(serviceUUID);
  if (!pService) {
    BLEGC_LOGE(LOG_TAG, "Service not found, service uuid: %s", std::string(serviceUUID).c_str());
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

  BLEGC_LOGE(LOG_TAG, "Characteristic not found, service uuid: %s, characteristic uuid: %s.",
             std::string(serviceUUID).c_str(),
             isNull(characteristicUUID) ? "null" : std::string(characteristicUUID).c_str());

  return nullptr;
}
};  // namespace blegc
