#pragma once

#include "BLEBaseEvent.h"

struct XboxBatteryEvent final : BLEBaseEvent {
  /// @brief Charge level of the controller's battery. Takes values between 0.0 and 1.0. A full battery yields 1.0.
  float level{};

  BLEDecodeResult decode(uint8_t data[], size_t dataLen) override;
};
