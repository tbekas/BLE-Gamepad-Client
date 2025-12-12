#include "BLEBaseController.h"

#define LOG_LOCAL_LEVEL 6

#include <NimBLEDevice.h>
#include "BLEGamepadClient.h"
#include "logger.h"
#include "utils.h"

BLEAbstractController::BLEAbstractController()
    : _pendingDeregistration(false),
      _address(0),
      _pClient(nullptr),
      _connectionState(ConnectionState::Disconnected),
      _lastAddress(NimBLEAddress()) {}

void BLEAbstractController::begin() {
  BLEGamepadClient::init();

  BLEGamepadClient::_controllerRegistry.registerController(this);
}

void BLEAbstractController::end() {
  auto valueOld = _pendingDeregistration.exchange(true);

  if (valueOld) {
    return;
  }

  BLEGamepadClient::_controllerRegistry.deregisterController(this);
}
/**
 * @brief Returns the address of the currently connected controller. If controller is not connected a null address
 * is returned (a NimBLEAddress instance for which method `isNull()` returns true).
 *
 * @return The relevant address based on the current connection state.
 */
NimBLEAddress BLEAbstractController::getAddress() const {
  return _decode(_address);
}

uint64_t BLEAbstractController::_encode(const NimBLEAddress& address) {
  auto val = static_cast<uint64_t>(address);
  *(reinterpret_cast<uint8_t*>(&val) + BLE_DEV_ADDR_LEN) = address.getType();
  return val;
}

NimBLEAddress BLEAbstractController::_decode(const uint64_t& address) {
  uint64_t val;
  memcpy(&val, &address, BLE_DEV_ADDR_LEN);
  uint8_t type = *(reinterpret_cast<const uint8_t*>(&address) + BLE_DEV_ADDR_LEN);
  return {address, type};
}

bool BLEAbstractController::tryAllocate(const NimBLEAddress address) {
  auto addressNew = _encode(address);
  auto addressOld = _address.load();
  do {
    if (addressOld != 0) {
      return false;
    }
  } while (!_address.compare_exchange_weak(addressOld, addressNew));

  return true;
}

bool BLEAbstractController::tryDeallocate() {
  auto addressOld = _address.load();
  do {
    if (addressOld == 0) {
      return false;
    }
  } while (!_address.compare_exchange_weak(addressOld, 0));

  _lastAddress = _decode(addressOld);
  return true;
}

/**
 * @brief Is controller connected to the board and fully initialized.
 * @return True if controller is connected and fully initialized, false otherwise.
 */
bool BLEAbstractController::isConnected() const {
  return _connectionState == ConnectionState::Connected;
}
bool BLEAbstractController::isConnecting() const {
  return _connectionState == ConnectionState::Connecting;
}

NimBLEAddress BLEAbstractController::getLastAddress() const {
  return _lastAddress;
}

bool BLEAbstractController::isAllocated() const {
  return _address != 0;
}

void BLEAbstractController::markCompletedDeregistration() {
  _pendingDeregistration.exchange(false);
}
void BLEAbstractController::markConnecting() {
  _connectionState = ConnectionState::Connecting;
}

void BLEAbstractController::markConnected() {
  _connectionState = ConnectionState::Connected;
}

void BLEAbstractController::markDisconnected() {
  _connectionState = ConnectionState::Disconnected;
}

bool BLEAbstractController::isPendingDeregistration() const {
  return _pendingDeregistration;
}

bool BLEAbstractController::hidInit(NimBLEClient* pClient) {
  if (!blegc::discoverAttributes(pClient)) {
    return false;
  }

  blegc::readDeviceInfo(pClient, &_deviceInfo);
  BLEGC_LOGD("%s", std::string(_deviceInfo).c_str());

  std::vector<uint8_t> buffer;
  blegc::readReportMap(pClient, &buffer);
  BLEGC_LOGD_BUFFER(buffer.data(), buffer.size());
  return true;
}

NimBLEClient* BLEAbstractController::getClient() const {
  return _pClient;
}
void BLEAbstractController::setClient(NimBLEClient* pClient) {
  _pClient = pClient;
}

void BLEAbstractController::disconnect() {
  if (isConnected()) {
    auto* pClient = getClient();
    if (pClient) {
      if (pClient->disconnect()) {
        BLEGC_LOGD("Disconnect command sent successfully");
      } else {
        BLEGC_LOGD("Disconnect command failed");
      }
    }
  } else {
    BLEGC_LOGD("Cannot disconnect controller that's not connected");
  }
}
