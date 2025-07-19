#pragma once

#include <stdint.h>

struct VibrationsCommand {
  /// @brief Power applied to the motor behind left trigger. Allowed values are between 0.0 and 1.0, where 1.0
  /// represents the full power.
  float leftTriggerMotor;

  /// @brief Power applied to the motor behind right trigger. Allowed values are between 0.0 and 1.0, where 1.0
  /// represents the full power.
  float rightTriggerMotor;

  /// @brief Power applied to the motor located in the left handle of the gamepad. This motor produces strong
  /// low-frequency vibrations. Allowed values are between 0.0 and 1.0, where 1.0 represents the full power.
  float leftMotor;

  /// @brief Power applied to the motor located in the right handle of the gamepad. This motor produces more subtle
  /// high-frequency vibrations. Allowed values are between 0.0 and 1.0, where 1.0 represents the full power.
  float rightMotor;

  /// @brief Vibration duration in milliseconds. Maximum allowed duration is 2550 ms (2.55 seconds). Rounded down to the
  /// closest multiple of 10 ms.
  uint32_t durationMs;

  /// @brief Pause duration in milliseconds. Maximum allowed duration is 2550 ms (2.55 seconds). Rounded down to the
  /// closest multiple of 10 ms.
  uint32_t pauseMs;

  /// @brief Number of vibration-pause cycles. Defaults to 1 cycle.
  uint8_t cycles;

  VibrationsCommand()
      : leftTriggerMotor(0.0f),
        rightTriggerMotor(0.0f),
        leftMotor(0.0f),
        rightMotor(0.0f),
        durationMs(0),
        pauseMs(0),
        cycles(1) {}
};
