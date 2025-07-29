#pragma once

#include <NimBLEAddress.h>

struct BLEBaseEvent {
  /// @brief Peer address of the controller that send this event.
  NimBLEAddress controllerAddress{};
};
