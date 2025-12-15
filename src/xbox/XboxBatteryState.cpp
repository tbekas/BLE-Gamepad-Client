#include "XboxBatteryState.h"

#include <NimBLEDevice.h>
#include "logger.h"

constexpr size_t batteryDataLen = 1;

BLEDecodeResult XboxBatteryState::decode(uint8_t data[], size_t dataLen) {
  if (dataLen != batteryDataLen) {
    BLEGC_LOGE("Expected %d bytes, was %d bytes", batteryDataLen, dataLen);
    return BLEDecodeResult::InvalidReport;
  }

  this->level = 0.01f * static_cast<float>(data[0]);
  return BLEDecodeResult::Success;
}
bool XboxBatteryState::operator==(const XboxBatteryState& rhs) const {
  return
    this->controllerAddress == rhs.controllerAddress &&
    this->level == rhs.level;
}
bool XboxBatteryState::operator!=(const XboxBatteryState& rhs) const {
  return !(*this == rhs);
}
