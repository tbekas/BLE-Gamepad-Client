#include "BLEBaseController.h"

#include <NimBLEDevice.h>
#include "BLEGamepadClient.h"
#include "utils.h"

const NimBLEAddress BLEAbstractController::_nullAddress = NimBLEAddress();

BLEAbstractController::BLEAbstractController()
    : _pendingDeregistration(false), _connected(false), _address(&_nullAddress), _lastAddress(NimBLEAddress()) {}

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
  return _connected;
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
  BLEGC_LOGD("%s", std::string(_deviceInfo).c_str());

  std::vector<uint8_t> buffer;
  blegc::readReportMap(pClient, &buffer);
#if CONFIG_BT_BLEGC_ENABLE_DEBUG_DATA
  BLEGC_LOGD_BUFFER(buffer.data(), buffer.size());
#endif

  return true;
}

NimBLEClient* BLEAbstractController::getClient() const {
  const auto address(*_address.load());
  if (address.isNull()) {
    return nullptr;
  }

  auto* pClient = NimBLEDevice::getClientByPeerAddress(address);
  if (!pClient) {
    BLEGC_LOGE("BLE client not found, address %s", std::string(address).c_str());
    return nullptr;
  }
  return pClient;
}

void BLEAbstractController::disconnect() {
  if (_connected) {
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
