#include "Controller.h"
#include "BLEGamepadClient.h"

Controller::Controller() : Controller(NimBLEAddress()) {}

Controller::Controller(NimBLEAddress address)
    : _pCtrl(nullptr),
      _allowedAddress(address),
      _onConnect([](NimBLEAddress) {}),
      _onDisconnect([](NimBLEAddress) {}),
      _onControlsUpdate([](ControlsEvent&) {}),
      _onControlsUpdateSet(false),
      _onBatteryUpdate([](BatteryEvent&) {}),
      _onBatteryUpdateSet(false) {}

bool Controller::begin() {
  if (!BLEGamepadClient::isInitialized()) {
    if (!BLEGamepadClient::init()) {
      return false;
    }
  }

  _pCtrl = BLEGamepadClient::createController(_allowedAddress);
  if (!_pCtrl) {
    return false;
  }

  _pCtrl->onConnect(_onConnect);
  _pCtrl->onDisconnect(_onDisconnect);
  if (_onControlsUpdateSet) {
    _pCtrl->getControls().onUpdate(_onControlsUpdate);
  }
  if (_onBatteryUpdateSet) {
    _pCtrl->getBattery().onUpdate(_onBatteryUpdate);
  }

  return true;
}

bool Controller::isConnected() const {
  if (_pCtrl) {
    return _pCtrl->isInitialized();
  }

  return false;
}

/**
 * @brief Sets the callback that will be invoked when the controller connects.
 * @param callback Reference to a callback function.
 */
void Controller::onConnect(const OnConnect& callback) {
  _onConnect = callback;
  if (_pCtrl) {
    _pCtrl->onConnect(_onConnect);
  }
}

/**
 * @brief Sets the callback that will be invoked when the controller disconnects.
 * @param callback Reference to a callback function.
 */
void Controller::onDisconnect(const OnDisconnect& callback) {
  _onDisconnect = callback;
  if (_pCtrl) {
    _pCtrl->onDisconnect(_onDisconnect);
  }
}

void Controller::readControls(ControlsEvent& event) const {
  if (_pCtrl) {
    _pCtrl->getControls().read(event);
  }
}
void Controller::onControlsUpdate(const OnControlsUpdate& callback) {
  _onControlsUpdateSet = true;
  _onControlsUpdate = callback;
  if (_pCtrl) {
    _pCtrl->getControls().onUpdate(_onControlsUpdate);
  }
}

void Controller::readBattery(BatteryEvent& event) const {
  if (_pCtrl) {
    _pCtrl->getBattery().read(event);
  }
}

void Controller::onBatteryUpdate(const OnBatteryUpdate& callback) {
  _onBatteryUpdateSet = true;
  _onBatteryUpdate = callback;
  if (_pCtrl) {
    _pCtrl->getBattery().onUpdate(_onBatteryUpdate);
  }
}

void Controller::writeVibrations(const VibrationsCommand& cmd) const {
  if (_pCtrl) {
    _pCtrl->getVibrations().write(cmd);
  }
}
