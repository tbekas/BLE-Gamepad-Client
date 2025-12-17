#include "utils.h"

#include <NimBLEDevice.h>
#include <string>

#include "logger.h"

namespace blegc {

bool isNull(const NimBLEUUID& uuid) {
  return uuid.bitSize() == 0;
}

std::string remoteSvcToStr(const NimBLERemoteService* pSvc) {
  std::string res = "Service: uuid: " + std::string(pSvc->getUUID());
  res += ", handle: ";
  res += std::to_string(pSvc->getHandle());
  return res;
}

void writeProperties(std::string& out, const uint8_t p, const std::string& sep) {
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

uint8_t getProperties(const NimBLERemoteCharacteristic* pChar) {
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

std::string remoteCharToStr(const NimBLERemoteCharacteristic* pChar) {
  std::string res = "Characteristic: uuid: " + std::string(pChar->getUUID());
  res += ", handle: ";
  res += std::to_string(pChar->getHandle());
  res += ", properties: [";
  writeProperties(res, getProperties(pChar), ", ");
  res += "]";

  return res;
}

bool discoverAttributes(NimBLEClient* pClient) {
  if (!pClient->discoverAttributes()) {
    BLEGC_LOGE("Failed to discover attributes, address %s", std::string(pClient->getPeerAddress()).c_str());
    return false;
  }

  for (auto& pService : pClient->getServices(false)) {
    BLEGC_LOGD("Discovered %s", remoteSvcToStr(pService).c_str());
    for (auto& pChar : pService->getCharacteristics(false)) {
      BLEGC_LOGD("Discovered %s", remoteCharToStr(pChar).c_str());
    }
  }
  return true;
}

NimBLERemoteCharacteristic* findCharacteristic(NimBLEClient* pClient,
                                               const NimBLEUUID& serviceUUID,
                                               const NimBLEUUID& characteristicUUID,
                                               const uint8_t properties,
                                               const uint8_t idx) {
  std::string propStr;
  writeProperties(propStr, properties, ", ");
  BLEGC_LOGD("Looking up for characteristic, service uuid: %s, characteristic uuid: %s, properties: [%s], idx: %d.",
             std::string(serviceUUID).c_str(),
             isNull(characteristicUUID) ? "null" : std::string(characteristicUUID).c_str(), propStr.c_str(), idx);

  auto* pService = pClient->getService(serviceUUID);
  if (!pService) {
    BLEGC_LOGE("Service not found, service uuid: %s", std::string(serviceUUID).c_str());
    return nullptr;
  }

  unsigned int _idx = idx;

  auto characteristics = pService->getCharacteristics(false);
  if (characteristics.empty()) {
    characteristics = pService->getCharacteristics(true);
  }

  for (auto* pChar : characteristics) {
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

  // TODO change this to Debug log & log error in client methods
  BLEGC_LOGE("Characteristic not found, service uuid: %s, characteristic uuid: %s, properties: [%s], idx: %d.",
             std::string(serviceUUID).c_str(),
             isNull(characteristicUUID) ? "null" : std::string(characteristicUUID).c_str(), propStr.c_str(), idx);

  return nullptr;
}

NimBLERemoteCharacteristic* findNotifiableCharacteristic(NimBLEClient* pClient,
                                                         const NimBLEUUID& serviceUUID,
                                                         const NimBLEUUID& characteristicUUID,
                                                         const uint8_t idx) {
  return findCharacteristic(pClient, serviceUUID, characteristicUUID, BLE_GATT_CHR_PROP_NOTIFY, idx);
}

NimBLERemoteCharacteristic* findReadableCharacteristic(NimBLEClient* pClient,
                                                       const NimBLEUUID& serviceUUID,
                                                       const NimBLEUUID& characteristicUUID,
                                                       const uint8_t idx) {
  return findCharacteristic(pClient, serviceUUID, characteristicUUID, BLE_GATT_CHR_PROP_READ, idx);
}

NimBLERemoteCharacteristic* findWritableCharacteristic(NimBLEClient* pClient,
                                                       const NimBLEUUID& serviceUUID,
                                                       const NimBLEUUID& characteristicUUID,
                                                       const uint8_t idx) {
  return findCharacteristic(pClient, serviceUUID, characteristicUUID, BLE_GATT_CHR_PROP_WRITE, idx);
}

void readReportMap(NimBLEClient* pClient, std::vector<uint8_t>* pReportMap) {
  auto* pChar = findReadableCharacteristic(pClient, hidSvcUUID, reportMapCharUUID);
  if (!pChar) {
    return;
  }

  const auto attValue = pChar->readValue();
  pReportMap->assign(attValue.begin(), attValue.end());
}

bool haveShortenedName(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  return pAdvertisedDevice->haveType(BLE_HS_ADV_TYPE_INCOMP_NAME);
}

std::string getShortenedName(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  return pAdvertisedDevice->getPayloadByType(BLE_HS_ADV_TYPE_INCOMP_NAME);
}

uint16_t getManufacturerId(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  auto mfgData = pAdvertisedDevice->getManufacturerData();
  configASSERT(mfgData.size() >= 2);
  uint16_t manufacturerId = (mfgData[1] << 8) + mfgData[0];  // low endian to native endian conversion
  return manufacturerId;
}

}  // namespace blegc
