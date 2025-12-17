#pragma once

#include "BLEBaseValue.h"

struct XboxBatteryState final : BLEBaseValue {
  /// @brief Charge level of the controller's battery. Takes values between 0.0 and 1.0. A full battery yields 1.0.
  float level{};

  BLEDecodeResult decode(uint8_t data[], size_t dataLen);
  bool operator==(const XboxBatteryState& rhs) const;
  bool operator!=(const XboxBatteryState& rhs) const;
};
