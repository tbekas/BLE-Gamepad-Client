#pragma once

#include <string>
#include "BatteryEvent.h"
#include "ControlsEvent.h"
#include "SignalConfig.h"

using ControlsConfig = SignalConfig<ControlsEvent>;
using BatteryConfig = SignalConfig<BatteryEvent>;

struct ControllerConfig {
  ControllerConfig() = default;

  std::string deviceName{};
  ControlsConfig controls{};
  BatteryConfig battery{};
};
