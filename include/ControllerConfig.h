#pragma once

#include <string>
#include "BatteryEvent.h"
#include "ControlsEvent.h"
#include "IncomingSignalConfig.h"
#include "OutgoingSignalConfig.h"
#include "VibrationsCommand.h"

using ControlsConfig = IncomingSignalConfig<ControlsEvent>;
using BatteryConfig = IncomingSignalConfig<BatteryEvent>;
using VibrationsConfig = OutgoingSignalConfig<VibrationsCommand>;

struct ControllerConfig {
  std::string deviceName{};
  ControlsConfig controls{};
  BatteryConfig battery{};
  VibrationsConfig vibrations{};
};
