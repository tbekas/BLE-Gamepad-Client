#include "BLEBaseController.h"

#include <NimBLEDevice.h>
#include "BLEGamepadClient.h"

static auto* LOG_TAG = "BLEBaseController";

BLEBaseController::BLEBaseController(const NimBLEAddress allowedAddress)
    : _pendingDeregistration(false),
      _address(),
      _allowedAddress(allowedAddress),
      _lastAddress(),
      _onConnect([] {}),
      _onDisconnect([] {}) {}

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

/**
 * @brief Is controller connected to the board and fully initialized.
 * @return True if controller is connected and fully initialized, false otherwise.
 */
bool BLEBaseController::isConnected() const {
  return _connected;
}

NimBLEAddress BLEBaseController::getLastAddress() const {
  return _lastAddress;
}

void BLEBaseController::setLastAddress(const NimBLEAddress address) {
  _lastAddress = address;
}

bool BLEBaseController::isAllocated() const {
  return !_address.isNull();
}

void BLEBaseController::markPendingDeregistration() {
  _pendingDeregistration = true;
}

void BLEBaseController::markCompletedDeregistration() {
  _pendingDeregistration = false;
}

void BLEBaseController::markConnected() {
  _connected = true;
}

void BLEBaseController::markDisconnected() {
  _connected = false;
}

bool BLEBaseController::isPendingDeregistration() const {
  return _pendingDeregistration;
}

void BLEBaseController::callOnConnect() const {
  _onConnect();
}

void BLEBaseController::callOnDisconnect() const {
  _onDisconnect();
}

NimBLEClient* BLEBaseController::getClient() const {
  if (_address.isNull()) {
    return nullptr;
  }

  auto* pClient = NimBLEDevice::getClientByPeerAddress(_address);
  if (!pClient) {
    BLEGC_LOGE(LOG_TAG, "BLE client not found, address %s", std::string(_address).c_str());
    return nullptr;
  }
  return pClient;
}

void BLEBaseController::disconnect() {
  if (_connected) {
    auto* pClient = getClient();
    if (pClient) {
      if (pClient->disconnect()) {
        BLEGC_LOGD(LOG_TAG, "Disconnect command sent successfully");
      } else {
        BLEGC_LOGD(LOG_TAG, "Disconnect command failed");
      }
    }
  } else {
    BLEGC_LOGD(LOG_TAG, "Cannot disconnect controller that's not connected");
  }
}
