#pragma once

#include "../BLEBaseEvent.h"
#include "../utils.h"

struct XboxBatteryEvent : BLEBaseEvent {
  /// @brief Charge level of the controller's battery. Takes values between 0.0 and 1.0. A full battery yields 1.0.
  float level{};

  static const blegc::BLEValueDecoder<XboxBatteryEvent> Decoder;
  static const blegc::BLECharacteristicSpec CharSpec;
};
