#include "BLEBaseController.h"

#include <NimBLEDevice.h>
#include "BLEGamepadClient.h"
#include "utils.h"

const NimBLEAddress BLEAbstractController::_nullAddress = NimBLEAddress();

BLEAbstractController::BLEAbstractController()
    : _pendingDeregistration(false),
      _connectionState(ConnectionState::Disconnected),
      _address(&_nullAddress),
      _pClient(nullptr),
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
  return {*_address.load()};
}

bool BLEAbstractController::tryAllocate(const NimBLEAddress address) {
  const auto* pAddressNew = new NimBLEAddress(address);
  const auto* pAddressOld = _address.load();
  do {
    if (!pAddressOld->isNull()) {
      delete pAddressNew;
      return false;
    }
  } while (!_address.compare_exchange_weak(pAddressOld, pAddressNew));

  return true;
}

bool BLEAbstractController::tryDeallocate() {
  const auto* pAddressOld = _address.load();
  do {
    if (pAddressOld->isNull()) {
      return false;
    }
  } while (!_address.compare_exchange_weak(pAddressOld, &_nullAddress));

  _lastAddress = NimBLEAddress(*pAddressOld);
  delete pAddressOld;
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
  return !_address.load()->isNull();
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
#if CONFIG_BT_BLEGC_ENABLE_DEBUG_DATA
  BLEGC_LOGD_BUFFER(buffer.data(), buffer.size());
#endif

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
