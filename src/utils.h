#pragma once

#include <NimBLEDevice.h>
#include <string>

namespace blegc {

constexpr uint16_t appearance(const uint16_t category, const uint8_t subcategory) {
  return category <<  6 | subcategory;
}

// https://bitbucket.org/bluetooth-SIG/public/src/main/assigned_numbers/uuids/service_uuids.yaml
constexpr uint16_t deviceInfoSvcUUID = 0x180a;
constexpr uint16_t hidSvcUUID = 0x1812;
constexpr uint16_t batterySvcUUID = 0x180f;

// https://bitbucket.org/bluetooth-SIG/public/src/main/assigned_numbers/uuids/characteristic_uuids.yaml
constexpr uint16_t hidInfoCharUUID = 0x2a4a;
constexpr uint16_t reportMapCharUUID = 0x2a4b;
constexpr uint16_t hidControlCharUUID = 0x2a4c;
constexpr uint16_t inputReportChrUUID = 0x2a4d;
constexpr uint16_t batteryLevelCharUUID = 0x2a19;
constexpr uint16_t batteryLevelDscUUID = 0x2904;
constexpr uint16_t manufacturerNameChrUUID = 0x2a29;
constexpr uint16_t modelNameChrUUID = 0x2a24;
constexpr uint16_t serialNumberChrUUID = 0x2a25;
constexpr uint16_t firmwareRevisionChrUUID = 0x2a26;
constexpr uint16_t pnpIdChrUUID = 0x2a50;

// https://bitbucket.org/bluetooth-SIG/public/src/main/assigned_numbers/core/appearance_values.yaml
constexpr uint16_t keyboardAppearance = appearance(0x00F, 0x01);
constexpr uint16_t mouseAppearance = appearance(0x00F, 0x02);
constexpr uint16_t joystickAppearance = appearance(0x00F, 0x03);
constexpr uint16_t gamepadAppearance = appearance(0x00F, 0x04);

// https://bitbucket.org/bluetooth-SIG/public/src/main/assigned_numbers/company_identifiers/company_identifiers.yaml
constexpr uint16_t microsoftCompanyId = 0x0006;
constexpr uint16_t valveCorporationCompanyId = 0x055D;

bool isNull(const NimBLEUUID& uuid);

std::string remoteSvcToStr(const NimBLERemoteService* pSvc);

void writeProperties(std::string& out, uint8_t p, const std::string& sep);

uint8_t getProperties(const NimBLERemoteCharacteristic* pChar);

std::string remoteCharToStr(const NimBLERemoteCharacteristic* pChar);

bool discoverAttributes(NimBLEClient* pClient);

NimBLERemoteCharacteristic* findCharacteristic(NimBLEClient* pClient,
                                                      const NimBLEUUID& serviceUUID,
                                                      const NimBLEUUID& characteristicUUID,
                                                      uint8_t properties,
                                                      uint8_t idx = 0);

NimBLERemoteCharacteristic* findNotifiableCharacteristic(NimBLEClient* pClient,
                                                                const NimBLEUUID& serviceUUID,
                                                                const NimBLEUUID& characteristicUUID,
                                                                uint8_t idx = 0);

NimBLERemoteCharacteristic* findReadableCharacteristic(NimBLEClient* pClient,
                                                              const NimBLEUUID& serviceUUID,
                                                              const NimBLEUUID& characteristicUUID,
                                                              uint8_t idx = 0);

NimBLERemoteCharacteristic* findWritableCharacteristic(NimBLEClient* pClient,
                                                              const NimBLEUUID& serviceUUID,
                                                              const NimBLEUUID& characteristicUUID,
                                                              uint8_t idx = 0);

void readReportMap(NimBLEClient* pClient, std::vector<uint8_t>* pReportMap);

bool haveShortenedName(const NimBLEAdvertisedDevice* pAdvertisedDevice);

std::string getShortenedName(const NimBLEAdvertisedDevice* pAdvertisedDevice);

uint16_t getManufacturerId(const NimBLEAdvertisedDevice* pAdvertisedDevice);

}  // namespace blegc
