#include "Controller.h"
#include <NimBLEDevice.h>
#include "BLEGamepadClient.h"
#include "Utils.h"

Controller::Controller() : Controller(NimBLEAddress()) {}

Controller::Controller(const std::string& address) : Controller(NimBLEAddress(address, BLE_ADDR_PUBLIC)) {}

Controller::Controller(const NimBLEAddress address)
    : _pCtrl(nullptr),
      _allowedAddress(address),
      _onConnect([](NimBLEAddress) {}),
      _onDisconnect([](NimBLEAddress) {}),
      _onControlsUpdate([](ControlsEvent&) {}),
      _onControlsUpdateIsSet(false),
      _onBatteryUpdate([](BatteryEvent&) {}),
      _onBatteryUpdateIsSet(false) {}
/**
 * @brief Initializes a controller instance and NimBLE stack if it's not already initialized.
 * @return True if initialization was successful.
 */
bool Controller::begin() {
  if (!BLEGamepadClient::isInitialized()) {
    if (!BLEGamepadClient::init()) {
      return false;
    }
  }

  _pCtrl = BLEGamepadClient::_createController(_allowedAddress);
  if (!_pCtrl) {
    return false;
  }

  _pCtrl->onConnect(_onConnect);
  _pCtrl->onDisconnect(_onDisconnect);
  if (_onControlsUpdateIsSet) {
    _pCtrl->getControls().onUpdate(_onControlsUpdate);
  }
  if (_onBatteryUpdateIsSet) {
    _pCtrl->getBattery().onUpdate(_onBatteryUpdate);
  }

  return true;
}

/**
 * @brief Is controller connected to the board.
 * @return True if controller is connected, false otherwise.
 */
bool Controller::isConnected() const {
  if (_pCtrl) {
    return _pCtrl->isInitialized();
  }

  return false;
}

/**
 * @brief Returns one of the following:
 *   - The address of the currently connected controller (if connected)
 *   - The address of the controller allowed to be assigned to this instance (if predefined via constructor param)
 *   - A null address (a NimBLEAddress instance for which method `isNull()` returns true)
 *
 * @return The relevant address based on the current connection or configuration state.
 */
NimBLEAddress Controller::getAddress() const {
  if (_pCtrl && _pCtrl->isInitialized()) {
    return _pCtrl->getAddress();
  }

  return _allowedAddress;
}

/**
 * @brief Sets the callback to be invoked when the controller connects.
 * @param callback Reference to a callback function.
 */
void Controller::onConnect(const OnConnect& callback) {
  _onConnect = callback;
  if (_pCtrl) {
    _pCtrl->onConnect(_onConnect);
  }
}

/**
 * @brief Sets the callback to be invoked when the controller disconnects.
 * @param callback Reference to the callback function.
 */
void Controller::onDisconnect(const OnDisconnect& callback) {
  _onDisconnect = callback;
  if (_pCtrl) {
    _pCtrl->onDisconnect(_onDisconnect);
  }
}

/**
 * @brief Read the controls state from the connected controller.
 * @param[out] event Reference to the event instance where the data will be written.
 */
void Controller::readControls(ControlsEvent& event) const {
  if (_pCtrl) {
    _pCtrl->getControls().read(event);
  }
}

/**
 * @brief Sets the callback to be invoked when the controller sends update to the controls state.
 * @param callback Reference to the callback function.
 */
void Controller::onControlsUpdate(const OnControlsUpdate& callback) {
  _onControlsUpdateIsSet = true;
  _onControlsUpdate = callback;
  if (_pCtrl) {
    _pCtrl->getControls().onUpdate(_onControlsUpdate);
  }
}

/**
 * @brief Read the battery state from the connected controller.
 * @param[out] event Reference to the event instance where the data will be written.
 */
void Controller::readBattery(BatteryEvent& event) const {
  if (_pCtrl) {
    _pCtrl->getBattery().read(event);
  }
}

/**
 * @brief Sets the callback to be invoked when the controller sends update to the battery state.
 * @param callback Reference to the callback function.
 */
void Controller::onBatteryUpdate(const OnBatteryUpdate& callback) {
  _onBatteryUpdateIsSet = true;
  _onBatteryUpdate = callback;
  if (_pCtrl) {
    _pCtrl->getBattery().onUpdate(_onBatteryUpdate);
  }
}

/**
 * @brief Send the vibrations command to the connected controller.
 * @param cmd Command enabling specific motors in the controller.
 */
void Controller::writeVibrations(const VibrationsCommand& cmd) const {
  if (_pCtrl) {
    _pCtrl->getVibrations().write(cmd);
  }
}
