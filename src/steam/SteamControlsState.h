#pragma once

#include "BLEBaseValue.h"

struct SteamControlsState final : BLEBaseValue {
  /// @brief Stick deflection along the X-axis. Takes values between -1.0 and 1.0. No deflection should yield 0.0,
  /// unless affected by stick drift. Positive values represent deflection to the right, and negative values to the
  /// left.
  float stickX{0.0f};

  /// @brief Stick deflection along the Y-axis. Takes values between -1.0 and 1.0. No deflection should yield 0.0,
  /// unless affected by stick drift. Positive values represent upward deflection, and negative values downward.
  float stickY{0.0f};

  /// @brief Button activated when pressing down on the stick, also known as the L3 button.
  bool stickButton{false};

  /// @brief Touch position along the left trackpad's X-axis. Ranges from -1.0 to 1.0, with 0.0 at the center. Positive
  /// values indicate touches on the right side, and negative values indicate touches on the left side.
  float leftPadX{0.0f};

  /// @brief Touch position along the left trackpad's Y-axis. Ranges from -1.0 to 1.0, with 0.0 at the center. Positive
  /// values indicate touches on the upper side, and negative values indicate touches on the lower side.
  float leftPadY{0.0f};

  /// @brief Touch position along the right trackpad's X-axis. Ranges from -1.0 to 1.0, with 0.0 at the center.
  /// Positive values indicate touches on the right side, and negative values indicate touches on the left side.
  float rightPadX{0.0f};

  /// @brief Touch position along the right trackpad's Y-axis. Ranges from -1.0 to 1.0, with 0.0 at the center.
  /// Positive values indicate touches on the upper side, and negative values indicate touches on the lower side.
  float rightPadY{0.0f};

  /// @brief Left trackpad click (press down on the trackpad).
  bool leftPadClick{false};

  /// @brief Right trackpad click (press down on the trackpad).
  bool rightPadClick{false};

  /// @brief True while the left trackpad is being touched.
  bool leftPadTouch{false};

  /// @brief True while the right trackpad is being touched.
  bool rightPadTouch{false};

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

  /// @brief Pressure level of a left trigger. Takes values between 0.0 and 1.0. No pressure should yield 0.0.
  float leftTrigger{0.0f};

  /// @brief Pressure level of a right trigger. Takes values between 0.0 and 1.0. No pressure should yield 0.0.
  float rightTrigger{0.0f};

  /// @brief Button activated when the left trigger is fully pressed.
  bool leftTriggerButton{false};

  /// @brief Button activated when the right trigger is fully pressed.
  bool rightTriggerButton{false};

  /// @brief Left grip button, located on the underside of the controller.
  bool leftGripButton{false};

  /// @brief Right grip button, located on the underside of the controller.
  bool rightGripButton{false};

  /// @brief The right-pointing button, located to the right of the Steam button.
  bool startButton{false};

  /// @brief The left-pointing button, located to the left of the Steam button.
  bool selectButton{false};

  /// @brief Steam button.
  bool steamButton{false};

  BLEDecodeResult decode(uint8_t data[], size_t dataLen);
  bool operator==(const SteamControlsState& rhs) const;
  bool operator!=(const SteamControlsState& rhs) const;
};
