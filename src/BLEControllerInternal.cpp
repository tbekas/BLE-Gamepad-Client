#include "BLEControllerInternal.h"
#include <NimBLEAddress.h>
#include "BLEControllerModel.h"
#include "BLEIncomingSignal.h"
#include "logger.h"
#include "utils.h"

BLEControllerInternal::BLEControllerInternal(const NimBLEAddress allowedAddress)
    : _initialized(false),
      _address(),
      _allowedAddress(allowedAddress),
      _lastAddress(),
      _onConnect([](NimBLEAddress) {}),
      _onDisconnect([](NimBLEAddress) {}) {}

bool BLEControllerInternal::init(BLEControllerModel& model) {
  if (_initialized) {
    return false;
  }

  if (!blegc::discoverAttributes(_address)) {
    return false;
  }

  if (model.controls.isEnabled()) {
    if (!_controls.init(_address, model.controls)) {
      return false;
    }
  }

  if (model.battery.isEnabled()) {
    if (!_battery.init(_address, model.battery)) {
      if (_controls.isInitialized()) {
        _controls.deinit(false);
      }
      return false;
    }
  }

  if (model.vibrations.isEnabled()) {
    if (!_vibrations.init(_address, model.vibrations)) {
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

BLEControlsSignal& BLEControllerInternal::getControls() {
  return _controls;
}

BLEBatterySignal& BLEControllerInternal::getBattery() {
  return _battery;
}

BLEVibrationsSignal& BLEControllerInternal::getVibrations() {
  return _vibrations;
}
