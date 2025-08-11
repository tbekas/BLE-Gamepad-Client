#pragma once

#include <string>
#include <list>
#include "BLEBatteryEvent.h"
#include "BLEControlsEvent.h"
#include "BLEIncomingSignal.h"
#include "BLEOutgoingSignal.h"
#include "BLEVibrationsCommand.h"

using BLEControlsSpec = BLEIncomingSignal<BLEControlsEvent>::Spec;
using BLEBatterySpec = BLEIncomingSignal<BLEBatteryEvent>::Spec;
using BLEVibrationsSpec = BLEOutgoingSignal<BLEVibrationsCommand>::Spec;

struct BLEControllerModel {
  std::string advertisedName{};
  std::list<BLEControlsSpec> controls{};
  std::list<BLEBatterySpec> battery{};
  BLEVibrationsSpec vibrations{};
};
