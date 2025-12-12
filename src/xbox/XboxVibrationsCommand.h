#pragma once

#include <cstddef>
#include "BLEBaseValue.h"

struct XboxVibrationsCommand final : BLEBaseValue {
  /// @brief Power applied to the motor behind left trigger. Allowed values are between 0.0 and 1.0, where 1.0
  /// represents the full power.
  float leftTriggerMotor{0.0f};

  /// @brief Power applied to the motor behind right trigger. Allowed values are between 0.0 and 1.0, where 1.0
  /// represents the full power.
  float rightTriggerMotor{0.0f};

  /// @brief Power applied to the motor located in the left handle of the gamepad. This motor produces strong
  /// low-frequency vibrations. Allowed values are between 0.0 and 1.0, where 1.0 represents the full power.
  float leftMotor{0.0f};

  /// @brief Power applied to the motor located in the right handle of the gamepad. This motor produces more subtle
  /// high-frequency vibrations. Allowed values are between 0.0 and 1.0, where 1.0 represents the full power.
  float rightMotor{0.0f};

  /// @brief Vibration duration in milliseconds. Maximum allowed duration is 2550 ms (2.55 seconds). Rounded down to the
  /// closest multiple of 10 ms.
  uint32_t durationMs{0};

  /// @brief Pause duration in milliseconds. Maximum allowed duration is 2550 ms (2.55 seconds). Rounded down to the
  /// closest multiple of 10 ms.
  uint32_t pauseMs{0};

  /// @brief Number of vibration-pause cycles. Defaults to 1 cycle.
  uint8_t cycles{1};

  BLEEncodeResult encode(size_t& usedBytes, uint8_t buffer[], size_t bufferLen);
};
