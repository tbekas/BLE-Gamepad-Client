#include "BLEControllerInternal.h"
#include <NimBLEAddress.h>
#include "BLEControllerAdapter.h"
#include "BLEIncomingSignal.h"
#include "BLEHelpers.h"
#include "logger.h"

BLEControllerInternal::BLEControllerInternal(const NimBLEAddress allowedAddress)
    : _initialized(false),
      _address(),
      _allowedAddress(allowedAddress),
      _lastAddress(),
      _onConnect([](NimBLEAddress) {}),
      _onDisconnect([](NimBLEAddress) {}) {}

bool BLEControllerInternal::init(BLEControllerAdapter& config) {
  if (_initialized) {
    return false;
  }

  if (!BLEHelpers::discoverAttributes(_address)) {
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

bool BLEControllerInternal::deinit(bool disconnected) {
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

bool BLEControllerInternal::isInitialized() const {
  return _initialized;
}

NimBLEAddress BLEControllerInternal::getAddress() const {
  return _address;
}

void BLEControllerInternal::setAddress(NimBLEAddress address) {
  _address = address;
}

NimBLEAddress BLEControllerInternal::getAllowedAddress() const {
  return _allowedAddress;
}

NimBLEAddress BLEControllerInternal::getLastAddress() const {
  return _lastAddress;
}

void BLEControllerInternal::setLastAddress(NimBLEAddress address) {
  _lastAddress = address;
}

void BLEControllerInternal::onConnect(const OnConnect& callback) {
  _onConnect = callback;
}

void BLEControllerInternal::onDisconnect(const OnDisconnect& callback) {
  _onDisconnect = callback;
}

ControlsSignal& BLEControllerInternal::getControls() {
  return _controls;
}

BatterySignal& BLEControllerInternal::getBattery() {
  return _battery;
}

VibrationsSignal& BLEControllerInternal::getVibrations() {
  return _vibrations;
}
