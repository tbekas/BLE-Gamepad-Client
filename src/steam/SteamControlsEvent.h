#pragma once

#include "../utils.h"

struct SteamControlsEvent {

  float stickX{0.0f};

  float stickY{0.0f};

  /// @brief Button activated when pressing down on the left stick, also known as the L3 button.
  bool stickButton{false};

  float rightPadX{0.0f};
  float rightPadY{0.0f};
  float leftPadX{0.0f};
  float leftPadY{0.0f};

  bool leftPadClick{false};
  bool rightPadClick{false};

  bool leftPadTouch{false}; // ?? if this is to stay, consider renaming "leftPadButton" to "leftPadClick" etc..
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

  /// @brief Left bumper button, also known as the L1 or L shoulder button.
  bool leftBumper{false};

  /// @brief Right bumper button, also known as the R1 or R shoulder button.
  bool rightBumper{false};

  /// @brief Pressure level of a left trigger. Takes values between 0.0 and 1.0. No pressure should yield 0.0. This
  /// control is also known as L2.
  float leftTrigger{0.0f};

  /// @brief Pressure level of a right trigger. Takes values between 0.0 and 1.0. No pressure should yield 0.0. This
  /// control is also known as R2.
  float rightTrigger{0.0f};

  bool leftTriggerButton{false};

  bool rightTriggerButton{false};

  /// @brief Left Grip. The button on the left underside of the controller.
  bool leftGripButton{false};

  /// @brief Right Grip. The button on the right underside of the controller.
  bool rightGripButton{false};

  bool startButton{false}; // right side

  bool systemButton{false}; // ??? idk

  bool selectButton{false}; // left side

  uint8_t data[128];
  size_t dataLen{0};

  static const blegc::BLEValueDecoder<SteamControlsEvent> Decoder;
  static const blegc::BLECharacteristicLocation CharacteristicLocation;
};
