#pragma once

#include <NimBLEUUID.h>
#include <string>
#include "BatteryEvent.h"
#include "ControlsEvent.h"
#include "Parser.h"
#include "SignalConfig.h"

using ControlsConfig = SignalConfig<ControlsEvent>;
using BatteryConfig = SignalConfig<BatteryEvent>;

struct ControllerConfig {
  ControllerConfig() = default;

  void setControlsConfig(NimBLEUUID serviceUUID, Parser<ControlsEvent> parser);
  void setControlsConfig(NimBLEUUID serviceUUID, NimBLEUUID characteristicUUID, Parser<ControlsEvent> parser);
  void setBatteryConfig(NimBLEUUID serviceUUID, Parser<BatteryEvent> parser);
  void setBatteryConfig(NimBLEUUID serviceUUID, NimBLEUUID characteristicUUID, Parser<BatteryEvent> parser);

  std::string deviceName{};
  ControlsConfig controlsConfig{};
  BatteryConfig batteryConfig{};
};
