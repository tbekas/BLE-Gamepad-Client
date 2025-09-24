#pragma once

#include "BLEBaseEvent.h"
#include "BLECharacteristicSpec.h"
#include "coders.h"

struct XboxBatteryEvent : BLEAbstractEvent {
  /// @brief Charge level of the controller's battery. Takes values between 0.0 and 1.0. A full battery yields 1.0.
  float level{};

  static const BLEValueDecoder<XboxBatteryEvent> Decoder;
  static const BLECharacteristicSpec CharSpec;
};
