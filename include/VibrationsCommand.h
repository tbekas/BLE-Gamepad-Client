#pragma once

#include<stdint.h>

struct VibrationsCommand {

  float leftTrigger;
  float rightTrigger;
  float leftMotor;
  float rightMotor;

  /// @brief Vibration duration in 0.01s units.
  uint8_t duration;

  /// @brief Pause duration in 0.01s units.
  uint8_t pause;

  /// @brief Number of vibration-pause cycles. Default is 1 cycle.
  uint8_t cycles;

  VibrationsCommand():
    leftTrigger(0.0f),
    rightTrigger(0.0f),
    leftMotor(0.0f),
    rightMotor(0.0f),
    duration(0),
    pause(0),
    cycles(1) {}
};
