#pragma once

#include <NimBLEAddress.h>

struct BaseEvent {
  /// @brief Peer address of the controller that send this event.
  NimBLEAddress controllerAddress;
};
