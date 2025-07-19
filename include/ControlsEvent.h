#pragma once

#include "BaseEvent.h"

struct ControlsEvent : BaseEvent {
  /**
   * @brief Stick deflection along an axis. Takes values between -1.0 and 1.0. No deflection should yield 0.0, unless
   * there is stick drift. Positive directions are up and right.
   *
   * This is a visual representation of the coordinate system used. The `o` represents the stick position when there is
   * no deflection.
   *
   * @verbatim
   *  1.0    /‾‾‾‾‾‾‾\
   *        /         \
   *  0.5  /           \
   *      |             |
   *  0.0 |      o      |
   *      |             |
   * -0.5  \           /
   *        \         /
   * -1.0    \_______/
   *     -1.0   0.0   1.0
   * @endverbatim
   */
  float leftStickX, leftStickY, rightStickX, rightStickY;

  /// @brief Button activated when pressing down on the left stick, also known as the L3 button.
  bool leftStickButton;

  /// @brief Button activated when pressing down on the right stick, also known as the R3 button.
  bool rightStickButton;

  /// @brief Up button on the directional pad.
  bool dpadUp;

  /// @brief Down button on the directional pad.
  bool dpadDown;

  /// @brief Left button on the directional pad.
  bool dpadLeft;

  /// @brief Right button on the directional pad.
  bool dpadRight;

  /// @brief Face button A, also known as the cross button.
  bool buttonA;

  /// @brief Face button B, also known as the circle button.
  bool buttonB;

  /// @brief Face button B, also known as the triangle button.
  bool buttonX;

  /// @brief Face button B, also known as the square button.
  bool buttonY;

  /// @brief Left bumper button, also known as the L1 or L shoulder button.
  bool leftBumper;

  /// @brief Right bumper button, also known as the R1 or R shoulder button.
  bool rightBumper;

  /// @brief Pressure level of a left trigger. Takes values between 0.0 and 1.0. No pressure should yield 0.0. This
  /// control is also known as L2.
  float leftTrigger;

  /// @brief Pressure level of a right trigger. Takes values between 0.0 and 1.0. No pressure should yield 0.0. This
  /// control is also known as R2.
  float rightTrigger;

  /// @brief Share button.
  bool share;

  /// @brief Menu button, also known as start button.
  bool menu;

  /// @brief View button, also known as back button.
  bool view;

  /// @brief Xbox button, also known as guide button.
  bool xbox;
};
