#include "BLEBaseController.h"

#include <NimBLEDevice.h>
#include "BLEGamepadClient.h"

static auto* LOG_TAG = "BLEBaseController";

BLEBaseController::BLEBaseController(const NimBLEAddress allowedAddress)
    : _connected(false),
      _address(),
      _allowedAddress(allowedAddress),
      _lastAddress(),
      _onConnect([](NimBLEAddress) {}),
      _onDisconnect([](NimBLEAddress) {}) {}

void BLEBaseController::begin() {
  BLEGamepadClient::initBLEDevice();

  BLEGamepadClient::_controllerRegistry.registerController(this);
}

void BLEBaseController::end() {
  BLEGamepadClient::_controllerRegistry.deregisterController(this);
}
/**
 * @brief Returns the address of the currently connected controller. If controller is not connected a null address
 * is returned (a NimBLEAddress instance for which method `isNull()` returns true).
 *
 * @return The relevant address based on the current connection state.
 */
NimBLEAddress BLEBaseController::getAddress() const {
  return _address;
}

void BLEBaseController::setAddress(const NimBLEAddress address) {
  _address = address;
}

NimBLEAddress BLEBaseController::getAllowedAddress() const {
  return _allowedAddress;
}

NimBLEAddress BLEBaseController::getLastAddress() const {
  return _lastAddress;
}

void BLEBaseController::setLastAddress(const NimBLEAddress address) {
  _lastAddress = address;
}

/**
 * @brief Is controller connected to the board.
 * @return True if controller is connected, false otherwise.
 */
bool BLEBaseController::isConnected() const {
  return _connected;
}

void BLEBaseController::setConnected() {
  _connected = true;
  _onConnect(_address);
}

void BLEBaseController::setDisconnected() {
  _connected = false;
  _onDisconnect(_address);
}

/**
 * @brief Sets the callback to be invoked when the controller connects.
 * @param onConnect Reference to a callback function.
 */
void BLEBaseController::onConnect(const OnConnect& onConnect) {
  _onConnect = onConnect;
}

/**
 * @brief Sets the callback to be invoked when the controller disconnects.
 * @param onDisconnect Reference to the callback function.
 */
void BLEBaseController::onDisconnect(const OnDisconnect& onDisconnect) {
  _onDisconnect = onDisconnect;
}

void BLEBaseController::disconnect() {
  if (!_connected) {
    BLEGC_LOGD(LOG_TAG, "Controller not connected, address %s", std::string(_address).c_str());
  }

  auto* pClient = NimBLEDevice::getClientByPeerAddress(_address);
  if (!pClient) {
    BLEGC_LOGE(LOG_TAG, "BLE client not found, address %s", std::string(_address).c_str());
    return;
  }

  pClient->disconnect();
}
