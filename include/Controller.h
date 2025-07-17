#pragma once

#include <NimBLEAddress.h>
#include "BatteryEvent.h"
#include "ControllerConfig.h"
#include "ControlsEvent.h"
#include "IncomingSignal.hpp"

using ControlsSignal = IncomingSignal<ControlsEvent>;
using BatterySignal = IncomingSignal<BatteryEvent>;

class Controller {
 public:
  explicit Controller(NimBLEAddress address);
  ~Controller() = default;

  NimBLEAddress getAddress() const;
  IncomingSignal<ControlsEvent>& controls();
  IncomingSignal<BatteryEvent>& battery();
  bool isConnected() const;
  bool isInitialized() const;

  bool init(ControllerConfig& config);
  bool deinit(bool disconnected);

 private:
  bool _initialized;
  NimBLEAddress _address;
  ControlsSignal _controlsSignal;
  BatterySignal _batterySignal;
};
