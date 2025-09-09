#pragma once

#include "../utils.h"

#include "../BLEBaseEvent.h"

struct SteamControlsEvent : BLEBaseEvent {

  /// @brief Stick deflection along the X-axis. Takes values between -1.0 and 1.0. No deflection should yield 0.0,
  /// unless affected by stick drift. Positive values represent deflection to the right, and negative values to the
  /// left.
  float stickX{0.0f};

  /// @brief Stick deflection along the Y-axis. Takes values between -1.0 and 1.0. No deflection should yield 0.0,
  /// unless affected by stick drift. Positive values represent upward deflection, and negative values downward.
  float stickY{0.0f};

  /// @brief Button activated when pressing down on the stick, also known as the L3 button.
  bool stickButton{false};


  float rightPadX{0.0f};
  float rightPadY{0.0f};
  float leftPadX{0.0f};
  float leftPadY{0.0f};

  bool leftPadClick{false};
  bool rightPadClick{false};

  bool leftPadTouch{false};
  bool rightPadTouch{false};

  /// @brief Up button on the directional pad.
  bool dpadUp{false};

  /// @brief Down button on the directional pad.
  bool dpadDown{false};

  /// @brief Left button on the directional pad.
  bool dpadLeft{false};

  /// @brief Right button on the directional pad.
  bool dpadRight{false};

  /// @brief Face button A, also known as the cross button.
  bool buttonA{false};

  /// @brief Face button B, also known as the circle button.
  bool buttonB{false};

  /// @brief Face button B, also known as the triangle button.
  bool buttonX{false};

  /// @brief Face button B, also known as the square button.
  bool buttonY{false};

  /// @brief Left bumper button, also known as the L1 or left shoulder button.
  bool leftBumper{false};

  /// @brief Right bumper button, also known as the R1 or right shoulder button.
  bool rightBumper{false};

  /// @brief Pressure level of a left trigger. Takes values between 0.0 and 1.0. No pressure should yield 0.0. This
  /// control is also known as L2.
  float leftTrigger{0.0f};

  /// @brief Pressure level of a right trigger. Takes values between 0.0 and 1.0. No pressure should yield 0.0. This
  /// control is also known as R2.
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

  bool systemButton{false};  // TODO keep it?

  static const blegc::BLEValueDecoder<SteamControlsEvent> Decoder;
  static const blegc::BLECharacteristicLocation CharacteristicLocation;
};
