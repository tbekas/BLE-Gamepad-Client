#pragma once

#include "BLEBaseEvent.h"

struct BLEBatteryEvent : BLEBaseEvent {
  /// @brief Charge level of the controller's battery. Takes values between 0.0 and 1.0. A full battery yields 1.0.
  float level;
};
