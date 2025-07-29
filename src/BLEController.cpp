#include "BLEController.h"
#include <NimBLEDevice.h>
#include "BLEControllerRegistry.h"

BLEController::BLEController() : BLEController(NimBLEAddress()) {}

BLEController::BLEController(const std::string& address) : BLEController(NimBLEAddress(address, BLE_ADDR_PUBLIC)) {}

BLEController::BLEController(const NimBLEAddress address)
    : _pCtrl(nullptr),
      _allowedAddress(address),
      _onConnect([](NimBLEAddress) {}),
      _onDisconnect([](NimBLEAddress) {}),
      _onControlsUpdate([](BLEControlsEvent&) {}),
      _onControlsUpdateIsSet(false),
      _onBatteryUpdate([](BLEBatteryEvent&) {}),
      _onBatteryUpdateIsSet(false) {}
/**
 * @brief Initializes a controller instance and NimBLE stack if it's not already initialized.
 * @return True if initialization was successful.
 */
bool BLEController::begin() {
  if (!BLEControllerRegistry::isInitialized()) {
    if (!BLEControllerRegistry::init()) {
      return false;
    }
  }

  if (_pCtrl) {
    return false;
  }

  _pCtrl = BLEControllerRegistry::_createController(_allowedAddress);
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
bool BLEController::isConnected() const {
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
NimBLEAddress BLEController::getAddress() const {
  if (_pCtrl && _pCtrl->isInitialized()) {
    return _pCtrl->getAddress();
  }

  return _allowedAddress;
}

/**
 * @brief Sets the callback to be invoked when the controller connects.
 * @param callback Reference to a callback function.
 */
void BLEController::onConnect(const OnConnect& callback) {
  _onConnect = callback;
  if (_pCtrl) {
    _pCtrl->onConnect(_onConnect);
  }
}

/**
 * @brief Sets the callback to be invoked when the controller disconnects.
 * @param callback Reference to the callback function.
 */
void BLEController::onDisconnect(const OnDisconnect& callback) {
  _onDisconnect = callback;
  if (_pCtrl) {
    _pCtrl->onDisconnect(_onDisconnect);
  }
}

/**
 * @brief Read the controls state from the connected controller.
 * @param[out] event Reference to the event instance where the data will be written.
 */
void BLEController::readControls(BLEControlsEvent& event) const {
  if (_pCtrl) {
    _pCtrl->getControls().read(event);
  }
}

/**
 * @brief Sets the callback to be invoked when the controller sends update to the controls state.
 * @param callback Reference to the callback function.
 */
void BLEController::onControlsUpdate(const OnControlsUpdate& callback) {
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
void BLEController::readBattery(BLEBatteryEvent& event) const {
  if (_pCtrl) {
    _pCtrl->getBattery().read(event);
  }
}

/**
 * @brief Sets the callback to be invoked when the controller sends update to the battery state.
 * @param callback Reference to the callback function.
 */
void BLEController::onBatteryUpdate(const OnBatteryUpdate& callback) {
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
void BLEController::writeVibrations(const BLEVibrationsCommand& cmd) const {
  if (_pCtrl) {
    _pCtrl->getVibrations().write(cmd);
  }
}
