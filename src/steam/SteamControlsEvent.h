#pragma once

#include "../utils.h"

struct SteamControlsEvent {
  /**
   * @brief Left stick deflection along the X-axis. Takes values between -1.0 and 1.0. No deflection should yield 0.0,
   * unless affected by stick drift. Positive values represent deflection to the right, and negative values to the left.
   *
   * This is a visual representation of the coordinate system used. The `o` represents the stick position when there is
   * no deflection.
   *
   @verbatim
    1.0    /‾‾‾‾‾‾‾\
          /         \
    0.5  /           \
        |             |
    0.0 |      o      |
        |             |
   -0.5  \           /
          \         /
   -1.0    \_______/
       -1.0   0.0   1.0
   @endverbatim
   */
  float leftStickX{0.0f};

  /**
   * @brief Left stick deflection along the Y-axis. Takes values between -1.0 and 1.0. No deflection should yield 0.0,
   * unless affected by stick drift. Positive values represent upward deflection, and negative values downward.
   *
   * @copydetails leftStickX
   */
  float leftStickY{0.0f};

  /// @brief Button activated when pressing down on the left stick, also known as the L3 button.
  bool leftStickButton{false};

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

  static const blegc::BLEValueDecoder<SteamControlsEvent> Decoder;
  static const blegc::BLECharacteristicLocation CharacteristicLocation;
};
