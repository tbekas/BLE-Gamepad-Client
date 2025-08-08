#pragma once

#include <string>
#include "BLEBatteryEvent.h"
#include "BLEControlsEvent.h"
#include "BLEIncomingSignal.h"
#include "BLEOutgoingSignal.h"
#include "BLEVibrationsCommand.h"

using BLEControlsAdapter = BLEIncomingSignal<BLEControlsEvent>::Adapter;
using BLEBatteryAdapter = BLEIncomingSignal<BLEBatteryEvent>::Adapter;
using BLEVibrationsAdapter = BLEOutgoingSignal<BLEVibrationsCommand>::Adapter;

struct BLEControllerAdapter {
  std::string deviceName{};
  BLEControlsAdapter controls{};
  BLEBatteryAdapter battery{};
  BLEVibrationsAdapter vibrations{};
};
