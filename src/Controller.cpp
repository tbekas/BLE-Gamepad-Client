#include "Controller.h"
#include <NimBLEAddress.h>
#include <NimBLEClient.h>
#include <NimBLEDevice.h>
#include <NimBLERemoteCharacteristic.h>
#include <NimBLERemoteService.h>
#include <NimBLEUUID.h>
#include <stdint.h>
#include <functional>
#include "BatteryEvent.h"
#include "ControllerConfig.h"
#include "ControlsEvent.h"
#include "Logger.h"
#include "Signal.hpp"
#include "Utils.h"

Controller::Controller(NimBLEAddress address) : _controlsSignal(address), _batterySignal(address) {
  _initialized = false;
  _address = address;
}

NimBLEAddress Controller::getAddress() {
  return _address;
}

Signal<ControlsEvent>& Controller::controls() {
  return _controlsSignal;
}

Signal<BatteryEvent>& Controller::battery() {
  return _batterySignal;
}

bool Controller::isInitialized() {
  return _initialized;
}

bool Controller::isConnected() {
  // initialization is kept in sync with client connect/client disconnect events
  return _initialized;
}

bool Controller::init(ControllerConfig& config) {
  if (_initialized) {
    return false;
  }

  if (config.controlsConfig.isEnabled()) {
    if (!_controlsSignal.init(config.controlsConfig)) {
      return false;
    }
  }

  if (config.batteryConfig.isEnabled()) {
    if (!_batterySignal.init(config.batteryConfig)) {
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
  result = _controlsSignal.deinit(disconnected) && result;
  result = _batterySignal.deinit(disconnected) && result;

  _initialized = false;
  return result;
}
