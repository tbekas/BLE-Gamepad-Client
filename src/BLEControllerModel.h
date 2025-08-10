#pragma once

#include <string>
#include "BLEBatteryEvent.h"
#include "BLEControlsEvent.h"
#include "BLEIncomingSignal.h"
#include "BLEOutgoingSignal.h"
#include "BLEVibrationsCommand.h"

using BLEControlsModel = BLEIncomingSignal<BLEControlsEvent>::Model;
using BLEBatteryModel = BLEIncomingSignal<BLEBatteryEvent>::Model;
using BLEVibrationsModel = BLEOutgoingSignal<BLEVibrationsCommand>::Model;

struct BLEControllerModel {
  std::string advertisedName{};
  BLEControlsModel controls{};
  BLEBatteryModel battery{};
  BLEVibrationsModel vibrations{};
};
