#pragma once

#include <functional>
#include <stdint.h>
#include <NimBLEAddress.h>
#include <NimBLERemoteCharacteristic.h>
#include "Signal.hpp"
#include "ControlsEvent.h"
#include "BatteryEvent.h"
#include "ControllerConfig.h"

class Controller {
  public:
    Controller(NimBLEAddress address);
    ~Controller() = default;

    NimBLEAddress getAddress();
    Signal<ControlsEvent>& controls();
    Signal<BatteryEvent>& battery();
    bool isConnected();
    bool isInitialized();

    bool init(ControllerConfig& config);
    bool deinit(bool disconnected);

  private:
    template<typename T>
    NimBLERemoteCharacteristic* _findCharacteristic(SignalConfig<T>& config);

    bool _initialized;
    NimBLEAddress _address;
    Signal<ControlsEvent> _controlsSignal;
    Signal<BatteryEvent> _batterySignal;
};
