#pragma once

#include <string>
#include "BLEBatteryEvent.h"
#include "BLEControlsEvent.h"
#include "BLEIncomingSignalAdapter.h"
#include "BLEOutgoingSignalAdapter.h"
#include "BLEVibrationsCommand.h"

using BLEControlsAdapter = BLEIncomingSignalAdapter<BLEControlsEvent>;
using BLEBatteryAdapter = BLEIncomingSignalAdapter<BLEBatteryEvent>;
using BLEVibrationsAdapter = BLEOutgoingSignalAdapter<BLEVibrationsCommand>;

struct BLEControllerAdapter {
  std::string deviceName{};
  BLEControlsAdapter controls{};
  BLEBatteryAdapter battery{};
  BLEVibrationsAdapter vibrations{};
};
