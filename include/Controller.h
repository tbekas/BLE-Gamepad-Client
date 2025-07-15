#pragma once

#include <NimBLEAddress.h>
#include <NimBLERemoteCharacteristic.h>
#include "BatteryEvent.h"
#include "ControllerConfig.h"
#include "ControlsEvent.h"
#include "Signal.hpp"

using ControlsSignal = Signal<ControlsEvent>;
using BatterySignal = Signal<BatteryEvent>;

class Controller {
  public:
    explicit Controller(NimBLEAddress address);
    ~Controller() = default;

    NimBLEAddress getAddress() const;
    Signal<ControlsEvent>& controls();
    Signal<BatteryEvent>& battery();
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
