#include "Controller.h"
#include <NimBLEAddress.h>
#include "ControllerConfig.h"
#include "IncomingSignal.h"
#include "Utils.h"

Controller::Controller(const NimBLEAddress address)
    : _initialized(false),
      _address(address),
      _controls(address),
      _battery(address),
      _vibrations(address) {}

NimBLEAddress Controller::getAddress() const {
  return _address;
}

ControlsSignal& Controller::controls() {
  return _controls;
}

BatterySignal& Controller::battery() {
  return _battery;
}

VibrationsSignal& Controller::vibrations() {
  return _vibrations;
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

  if (!Utils::discoverAttributes(_address)) {
    return false;
  }

  if (config.controls.isEnabled()) {
    if (!_controls.init(config.controls)) {
      return false;
    }
  }

  if (config.battery.isEnabled()) {
    if (!_battery.init(config.battery)) {
      if (_controls.isInitialized()) {
        _controls.deinit(false);
      }
      return false;
    }
  }

  if (config.vibrations.isEnabled()) {
    if (!_vibrations.init(config.vibrations)) {
      if (_battery.isInitialized()) {
        _battery.deinit(false);
      }
      if (_controls.isInitialized()) {
        _controls.deinit(false);
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
  result = _vibrations.deinit(disconnected) && result;
  result = _battery.deinit(disconnected) && result;
  result = _controls.deinit(disconnected) && result;

  _initialized = false;
  return result;
}
