#pragma once

struct VibrationsCommand {
  bool selectCenter, selectLeft, selectRight, selectShake;

  float powerCenter, powerLeft, powerRight, powerShake;

  uint8_t timeActiveMs;
  uint8_t timeInactiveMs;

  uint8_t repeat;
};
