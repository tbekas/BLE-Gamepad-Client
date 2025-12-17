#pragma once

#include "BLEBaseValue.h"

struct XboxControlsState final : BLEBaseValue {
  /// @brief Left stick deflection along the X-axis. Takes values between -1.0 and 1.0. No deflection should yield 0.0,
  /// unless affected by stick drift. Positive values represent deflection to the right, and negative values to the
  /// left.
  float leftStickX{0.0f};

  /// @brief Left stick deflection along the Y-axis. Takes values between -1.0 and 1.0. No deflection should yield 0.0,
  /// unless affected by stick drift. Positive values represent upward deflection, and negative values downward.
  float leftStickY{0.0f};

  /// @brief Right stick deflection along the X-axis. Takes values between -1.0 and 1.0. No deflection should yield 0.0,
  /// unless affected by stick drift. Positive values represent deflection to the right, and negative values to the
  /// left.
  float rightStickX{0.0f};

  /// @brief Right stick deflection along the Y-axis. Takes values between -1.0 and 1.0. No deflection should yield 0.0,
  /// unless affected by stick drift. Positive values represent upward deflection, and negative values downward.
  float rightStickY{0.0f};

  /// @brief Button activated when pressing down on the left stick.
  bool leftStickButton{false};

  /// @brief Button activated when pressing down on the right stick.
  bool rightStickButton{false};

  /// @brief Up button on the directional pad.
  bool dpadUp{false};

  /// @brief Down button on the directional pad.
  bool dpadDown{false};

  /// @brief Left button on the directional pad.
  bool dpadLeft{false};

  /// @brief Right button on the directional pad.
  bool dpadRight{false};

  /// @brief Face button A.
  bool buttonA{false};

  /// @brief Face button B.
  bool buttonB{false};

  /// @brief Face button X.
  bool buttonX{false};

  /// @brief Face button Y.
  bool buttonY{false};

  /// @brief Left bumper button, also known as the left shoulder button.
  bool leftBumper{false};

  /// @brief Right bumper button, also known as the right shoulder button.
  bool rightBumper{false};

  /// @brief Pressure level on the left trigger. Takes values between 0.0 and 1.0. No pressure should yield 0.0.
  float leftTrigger{0.0f};

  /// @brief Pressure level on the right trigger. Takes values between 0.0 and 1.0. No pressure should yield 0.0.
  float rightTrigger{0.0f};

  /// @brief Share button, located below the Xbox button in the center (model 1914).
  bool shareButton{false};

  /// @brief Menu button, located below the Xbox button on the right side.
  bool menuButton{false};

  /// @brief View button, located below the Xbox button on the left side.
  bool viewButton{false};

  /// @brief Xbox button.
  bool xboxButton{false};

  BLEDecodeResult decode(uint8_t data[], size_t dataLen);
  bool operator==(const XboxControlsState& rhs) const;
  bool operator!=(const XboxControlsState& rhs) const;
};
