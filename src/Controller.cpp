#include "Controller.h"
#include <NimBLEAddress.h>
#include "ControllerConfig.h"
#include "IncomingSignal.h"

Controller::Controller(const NimBLEAddress address)
    : _initialized(false),
      _address(address),
      _controlsSignal(address),
      _batterySignal(address),
      _vibrationsSignal(address) {}

NimBLEAddress Controller::getAddress() const {
  return _address;
}

ControlsSignal& Controller::controls() {
  return _controlsSignal;
}

BatterySignal& Controller::battery() {
  return _batterySignal;
}

VibrationsSignal& Controller::vibrations() {
  return _vibrationsSignal;
}

bool Controller::isInitialized() const {
  return _initialized;
}

bool Controller::isConnected() const {
  // initialization is kept in sync with client connect/client disconnect events
  return _initialized;
}

bool Controller::init(ControllerConfig& config) {
  if (_initialized) {
    return false;
  }

  if (config.controls.isEnabled()) {
    if (!_controlsSignal.init(config.controls)) {
      return false;
    }
  }

  if (config.battery.isEnabled()) {
    if (!_batterySignal.init(config.battery)) {
      if (_controlsSignal.isInitialized()) {
        _controlsSignal.deinit(false);
      }
      return false;
    }
  }

  if (config.vibrations.isEnabled()) {
    if (!_vibrationsSignal.init(config.vibrations)) {
      if (_batterySignal.isInitialized()) {
        _batterySignal.deinit(false);
      }
      if (_controlsSignal.isInitialized()) {
        _controlsSignal.deinit(false);
      }
      return false;
    }
  }

  _initialized = true;
  return true;
}

bool Controller::deinit(bool disconnected) {
  if (!_initialized) {
    return false;
  }
  bool result = true;

  // order of operands on the && matters here
  result = _vibrationsSignal.deinit(disconnected) && result;
  result = _batterySignal.deinit(disconnected) && result;
  result = _controlsSignal.deinit(disconnected) && result;

  _initialized = false;
  return result;
}
