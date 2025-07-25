#include "ControllerInternal.h"
#include <NimBLEAddress.h>
#include "ControllerConfig.h"
#include "IncomingSignal.h"
#include "Utils.h"
#include "logger.h"

ControllerInternal::ControllerInternal(const NimBLEAddress allowedAddress)
    : _initialized(false),
      _address(),
      _allowedAddress(allowedAddress),
      _lastAddress(),
      _onConnect([](NimBLEAddress) {}),
      _onDisconnect([](NimBLEAddress) {}) {}

bool ControllerInternal::init(ControllerConfig& config) {
  if (_initialized) {
    return false;
  }

  if (!Utils::discoverAttributes(_address)) {
    return false;
  }

  if (config.controls.isEnabled()) {
    if (!_controls.init(_address, config.controls)) {
      return false;
    }
  }

  if (config.battery.isEnabled()) {
    if (!_battery.init(_address, config.battery)) {
      if (_controls.isInitialized()) {
        _controls.deinit(false);
      }
      return false;
    }
  }

  if (config.vibrations.isEnabled()) {
    if (!_vibrations.init(_address, config.vibrations)) {
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
  _onConnect(_address);
  return true;
}

bool ControllerInternal::deinit(bool disconnected) {
  if (!_initialized) {
    return false;
  }
  bool result = true;

  // order of operands on the && operator matters here
  result = _vibrations.deinit(disconnected) && result;
  result = _battery.deinit(disconnected) && result;
  result = _controls.deinit(disconnected) && result;

  _initialized = false;
  _onDisconnect(_address);
  return result;
}

bool ControllerInternal::isInitialized() const {
  return _initialized;
}

NimBLEAddress ControllerInternal::getAddress() const {
  return _address;
}

void ControllerInternal::setAddress(NimBLEAddress address) {
  _address = address;
}

NimBLEAddress ControllerInternal::getAllowedAddress() const {
  return _allowedAddress;
}

NimBLEAddress ControllerInternal::getLastAddress() const {
  return _lastAddress;
}

void ControllerInternal::setLastAddress(NimBLEAddress address) {
  _lastAddress = address;
}

void ControllerInternal::onConnect(const OnConnect& callback) {
  _onConnect = callback;
}

void ControllerInternal::onDisconnect(const OnDisconnect& callback) {
  _onDisconnect = callback;
}

ControlsSignal& ControllerInternal::getControls() {
  return _controls;
}

BatterySignal& ControllerInternal::getBattery() {
  return _battery;
}

VibrationsSignal& ControllerInternal::getVibrations() {
  return _vibrations;
}
