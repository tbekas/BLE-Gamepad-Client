#include "BLEControllerInternal.h"
#include <NimBLEAddress.h>
#include <NimBLEDevice.h>
#include "BLEControllerModel.h"
#include "BLEIncomingSignal.h"
#include "logger.h"
#include "utils.h"

static auto* LOG_TAG = "BLEControllerInternal";

BLEControllerInternal::BLEControllerInternal(const NimBLEAddress allowedAddress)
    : _initialized(false),
      _address(),
      _allowedAddress(allowedAddress),
      _lastAddress(),
      _onConnect([](NimBLEAddress) {}),
      _onDisconnect([](NimBLEAddress) {}) {}

bool BLEControllerInternal::init(const BLEControllerModel& model) {
  if (_initialized) {
    return false;
  }

  auto* pClient = NimBLEDevice::getClientByPeerAddress(_address);
  if (!pClient) {
    BLEGC_LOGE(LOG_TAG, "BLE client not found, address %s", std::string(_address).c_str());
    return false;
  }

  if (!blegc::discoverAttributes(pClient)) {
    return false;
  }

  if (model.controls.enabled) {
    if (!_controls.init(pClient, model.controls)) {
      return false;
    }
  }

  if (model.battery.enabled) {
    if (!_battery.init(pClient, model.battery)) {
      if (_controls.isInitialized()) {
        _controls.deinit(false);
      }
      return false;
    }
  }

  if (model.vibrations.enabled) {
    if (!_vibrations.init(pClient, model.vibrations)) {
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
