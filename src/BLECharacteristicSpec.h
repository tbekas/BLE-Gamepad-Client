#pragma once

#include <NimBLEUUID.h>

namespace blegc {
static constexpr uint16_t deviceInfoSvcUUID = 0x180a;
static constexpr uint16_t hidSvcUUID = 0x1812;
static constexpr uint16_t batterySvcUUID = 0x180f;
static constexpr uint16_t hidInfoCharUUID = 0x2a4a;
static constexpr uint16_t reportMapCharUUID = 0x2a4b;
static constexpr uint16_t hidControlCharUUID = 0x2a4c;
static constexpr uint16_t inputReportChrUUID = 0x2a4d;
static constexpr uint16_t batteryLevelCharUUID = 0x2a19;
static constexpr uint16_t batteryLevelDscUUID = 0x2904;
}  // namespace blegc

struct BLECharacteristicSpec {
  NimBLEUUID serviceUUID;
  NimBLEUUID characteristicUUID;
  uint8_t properties;
  uint8_t idx;
};
