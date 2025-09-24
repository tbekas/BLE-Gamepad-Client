#pragma once

#include <NimBLEDevice.h>
#include <string>
#include "logger.h"

namespace blegc {

static auto* UTILS_LOG_TAG = "utils";

static constexpr uint16_t deviceInfoSvcUUID = 0x180a;
static constexpr uint16_t hidSvcUUID = 0x1812;
static constexpr uint16_t batterySvcUUID = 0x180f;
static constexpr uint16_t hidInfoCharUUID = 0x2a4a;
static constexpr uint16_t reportMapCharUUID = 0x2a4b;
static constexpr uint16_t hidControlCharUUID = 0x2a4c;
static constexpr uint16_t inputReportChrUUID = 0x2a4d;
static constexpr uint16_t batteryLevelCharUUID = 0x2a19;
static constexpr uint16_t batteryLevelDscUUID = 0x2904;

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
    BLEGC_LOGE(UTILS_LOG_TAG, "Failed to discover attributes, address %s",
               std::string(pClient->getPeerAddress()).c_str());
    return false;
  }

#if CONFIG_BT_BLEGC_LOG_LEVEL >= 4
  for (auto& pService : pClient->getServices(false)) {
    BLEGC_LOGD(UTILS_LOG_TAG, "Discovered %s", remoteSvcToStr(pService).c_str());
    for (auto& pChar : pService->getCharacteristics(false)) {
      BLEGC_LOGD(UTILS_LOG_TAG, "Discovered %s", remoteCharToStr(pChar).c_str());
    }
  }
#endif
  return true;
}

static NimBLERemoteCharacteristic* findCharacteristic(NimBLEClient* pClient,
                                                      const NimBLEUUID& serviceUUID,
                                                      const NimBLEUUID& characteristicUUID,
                                                      const uint8_t properties,
                                                      const uint8_t idx = 0) {
  std::string propStr;
  writeProperties(propStr, properties, ", ");
  BLEGC_LOGD(UTILS_LOG_TAG,
             "Looking up for characteristic, service uuid: %s, characteristic uuid: %s, properties: [%s], idx: %d.",
             std::string(serviceUUID).c_str(),
             isNull(characteristicUUID) ? "null" : std::string(characteristicUUID).c_str(), propStr.c_str(), idx);

  auto* pService = pClient->getService(serviceUUID);
  if (!pService) {
    BLEGC_LOGE(UTILS_LOG_TAG, "Service not found, service uuid: %s", std::string(serviceUUID).c_str());
    return nullptr;
  }

  unsigned int _idx = idx;

  for (auto* pChar : pService->getCharacteristics(false)) {
    if (!isNull(characteristicUUID) && characteristicUUID != pChar->getUUID()) {
      continue;
    }

    if ((getProperties(pChar) & properties) != properties) {
      continue;
    }

    if (_idx-- != 0) {
      continue;
    }

    return pChar;
  }

  BLEGC_LOGE(UTILS_LOG_TAG,
             "Characteristic not found, service uuid: %s, characteristic uuid: %s, properties: [%s], idx: %d.",
             std::string(serviceUUID).c_str(),
             isNull(characteristicUUID) ? "null" : std::string(characteristicUUID).c_str(), propStr.c_str(), idx);

  return nullptr;
}

static NimBLERemoteCharacteristic* findNotifiableCharacteristic(NimBLEClient* pClient,
                                                                const NimBLEUUID& serviceUUID,
                                                                const NimBLEUUID& characteristicUUID,
                                                                const uint8_t idx = 0) {
  return findCharacteristic(pClient, serviceUUID, characteristicUUID, BLE_GATT_CHR_PROP_NOTIFY, idx);
}

static NimBLERemoteCharacteristic* findWritableCharacteristic(NimBLEClient* pClient,
                                                              const NimBLEUUID& serviceUUID,
                                                              const NimBLEUUID& characteristicUUID,
                                                              const uint8_t idx = 0) {
  return findCharacteristic(pClient, serviceUUID, characteristicUUID, BLE_GATT_CHR_PROP_WRITE, idx);
}

};  // namespace blegc
