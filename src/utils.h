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

static void writeProperties(std::string& out, const uint8_t p, const std::string& sep) {
  // clang-format off
  if (p & BLE_GATT_CHR_PROP_BROADCAST) { out += "broadcast"; out += sep; }
  if (p & BLE_GATT_CHR_PROP_READ) { out += "read"; out += sep; }
  if (p & BLE_GATT_CHR_PROP_WRITE_NO_RSP) { out += "writeNoResponse"; out += sep; }
  if (p & BLE_GATT_CHR_PROP_WRITE) { out += "write"; out += sep; }
  if (p & BLE_GATT_CHR_PROP_NOTIFY) { out += "notify"; out += sep; }
  if (p & BLE_GATT_CHR_PROP_INDICATE) { out += "indicate"; out += sep; }
  if (p & BLE_GATT_CHR_PROP_AUTH_SIGN_WRITE) { out += "writeSigned"; out += sep; }
  if (p & BLE_GATT_CHR_PROP_EXTENDED) { out += "hasExtendedProps"; out += sep; }
  // clang-format on

  auto pos = out.rfind(sep);
  if (pos == out.size() - sep.length()) {
    out.erase(pos);
  }
}

static uint8_t getProperties(const NimBLERemoteCharacteristic* pChar) {
  uint8_t res = 0;
  // clang-format off
  if (pChar->canBroadcast()) { res |= BLE_GATT_CHR_PROP_BROADCAST; }
  if (pChar->canRead()) { res |= BLE_GATT_CHR_PROP_READ; }
  if (pChar->canWriteNoResponse()) { res |= BLE_GATT_CHR_PROP_WRITE_NO_RSP; }
  if (pChar->canWrite()) { res |= BLE_GATT_CHR_PROP_WRITE; }
  if (pChar->canNotify()) { res |= BLE_GATT_CHR_PROP_NOTIFY; }
  if (pChar->canIndicate()) { res |= BLE_GATT_CHR_PROP_INDICATE; }
  if (pChar->canWriteSigned()) { res |= BLE_GATT_CHR_PROP_AUTH_SIGN_WRITE; }
  if (pChar->hasExtendedProps()) { res |= BLE_GATT_CHR_PROP_EXTENDED; }
  // clang-format on
  return res;
}

static std::string remoteCharToStr(const NimBLERemoteCharacteristic* pChar) {
  std::string res = "Characteristic: uuid: " + std::string(pChar->getUUID());
  res += ", handle: ";
  res += std::to_string(pChar->getHandle());
  res += ", properties: [";
  writeProperties(res, getProperties(pChar), ", ");
  res += "]";

  return res;
}

static bool discoverAttributes(NimBLEClient* pClient) {
  if (!pClient->discoverAttributes()) {
    BLEGC_LOGE(LOG_TAG, "Failed to discover attributes, address %s", std::string(pClient->getPeerAddress()).c_str());
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

static NimBLERemoteCharacteristic* findCharacteristic(NimBLEClient* pClient,
                                                      const NimBLEUUID& serviceUUID,
                                                      const NimBLEUUID& characteristicUUID,
                                                      const uint8_t properties = 0xff,
                                                      const unsigned int idx = 0) {
  std::string propStr;
  writeProperties(propStr, properties, ", ");
  BLEGC_LOGD(LOG_TAG,
             "Looking up for characteristic, service uuid: %s, characteristic uuid: %s, properties: [%s], idx: %d.",
             std::string(serviceUUID).c_str(),
             isNull(characteristicUUID) ? "null" : std::string(characteristicUUID).c_str(), propStr.c_str(), idx);

  auto* pService = pClient->getService(serviceUUID);
  if (!pService) {
    BLEGC_LOGE(LOG_TAG, "Service not found, service uuid: %s", std::string(serviceUUID).c_str());
    return nullptr;
  }

  unsigned int _idx = idx;

  for (auto* pChar : pService->getCharacteristics(false)) {
    if (!isNull(characteristicUUID) && characteristicUUID != pChar->getUUID()) {
      continue;
    }

    if (!(getProperties(pChar) & properties)) {
      continue;
    }

    if (_idx-- != 0) {
      continue;
    }

    return pChar;
  }

  BLEGC_LOGE(LOG_TAG, "Characteristic not found, service uuid: %s, characteristic uuid: %s, properties: [%s], idx: %d.",
             std::string(serviceUUID).c_str(),
             isNull(characteristicUUID) ? "null" : std::string(characteristicUUID).c_str(), propStr.c_str(), idx);

  return nullptr;
}
};  // namespace blegc
