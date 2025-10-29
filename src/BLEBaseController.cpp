#include "BLEBaseController.h"

#include <NimBLEDevice.h>
#include "BLEGamepadClient.h"
#include "utils.h"

static auto* LOG_TAG = "BLEBaseController";

BLEAbstractController::BLEAbstractController(const NimBLEAddress allowedAddress)
    : _pendingDeregistration(false),
      _address(),
      _allowedAddress(allowedAddress),
      _lastAddress() {}

void BLEAbstractController::begin() {
  BLEGamepadClient::initBLEDevice();

  BLEGamepadClient::_controllerRegistry.registerController(this);
}

void BLEAbstractController::end() {
  BLEGamepadClient::_controllerRegistry.deregisterController(this);
}
/**
 * @brief Returns the address of the currently connected controller. If controller is not connected a null address
 * is returned (a NimBLEAddress instance for which method `isNull()` returns true).
 *
 * @return The relevant address based on the current connection state.
 */
NimBLEAddress BLEAbstractController::getAddress() const {
  return _address;
}

void BLEAbstractController::setAddress(const NimBLEAddress address) {
  _address = address;
}

NimBLEAddress BLEAbstractController::getAllowedAddress() const {
  return _allowedAddress;
}

/**
 * @brief Is controller connected to the board and fully initialized.
 * @return True if controller is connected and fully initialized, false otherwise.
 */
bool BLEAbstractController::isConnected() const {
  return _connected;
}

NimBLEAddress BLEAbstractController::getLastAddress() const {
  return _lastAddress;
}

void BLEAbstractController::setLastAddress(const NimBLEAddress address) {
  _lastAddress = address;
}

bool BLEAbstractController::isAllocated() const {
  return !_address.isNull();
}

void BLEAbstractController::markPendingDeregistration() {
  _pendingDeregistration = true;
}

void BLEAbstractController::markCompletedDeregistration() {
  _pendingDeregistration = false;
}

void BLEAbstractController::markConnected() {
  _connected = true;
}

void BLEAbstractController::markDisconnected() {
  _connected = false;
}

bool BLEAbstractController::isPendingDeregistration() const {
  return _pendingDeregistration;
}

bool BLEAbstractController::hidInit(NimBLEClient* pClient) {
  if (!blegc::discoverAttributes(pClient)) {
    return false;
  }

  blegc::readDeviceInfo(pClient, &_deviceInfo);
  BLEGC_LOGD(LOG_TAG, "%s", std::string(_deviceInfo).c_str());

  std::vector<uint8_t> buffer;
  blegc::readReportMap(pClient, &buffer);
#if CONFIG_BT_BLEGC_ENABLE_DEBUG_DATA
  printHexdump(buffer.data(), buffer.size());
#endif

  return true;
}

NimBLEClient* BLEAbstractController::getClient() const {
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

void BLEAbstractController::disconnect() {
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
