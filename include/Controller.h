#pragma once

#include <NimBLEAddress.h>
#include "BatteryEvent.h"
#include "ControllerConfig.h"
#include "ControlsEvent.h"
#include "IncomingSignal.h"
#include "OutgoingSignal.h"

class Controller {
 public:
  explicit Controller(NimBLEAddress address);
  ~Controller() = default;

  NimBLEAddress getAddress() const;
  ControlsSignal& controls();
  BatterySignal& battery();
  VibrationsSignal& vibrations();
  bool isConnected() const;
  bool isInitialized() const;

  bool init(ControllerConfig& config);
  bool deinit(bool disconnected);

 private:
  bool _initialized;
  NimBLEAddress _address;
  ControlsSignal _controlsSignal;
  BatterySignal _batterySignal;
  VibrationsSignal _vibrationsSignal;
};
