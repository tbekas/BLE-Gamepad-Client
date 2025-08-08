#include "BLEGamepadClient.h"

static auto* LOG_TAG = "BLEGamepadClient";

bool BLEGamepadClient::_initialized(false);
TaskHandle_t BLEGamepadClient::_autoScanTask;
BLEControllerAdapterRegistry BLEGamepadClient::_adapterRegistry;
BLEControllerRegistry BLEGamepadClient::_controllerRegistry(_autoScanTask, _adapterRegistry);
BLEAutoScanner BLEGamepadClient::_autoScanner(_autoScanTask, _controllerRegistry, _adapterRegistry);

bool BLEGamepadClient::init() {
  if (_initialized) {
    return false;
  }

  BLEGC_LOGD(LOG_TAG, "Initializing services");

  if (!NimBLEDevice::isInitialized()) {
    NimBLEDevice::init(CONFIG_BT_BLEGC_DEVICE_NAME);
    NimBLEDevice::setPower(CONFIG_BT_BLEGC_POWER_DBM);
    NimBLEDevice::setSecurityAuth(CONFIG_BT_BLEGC_SECURITY_AUTH);
    NimBLEDevice::setSecurityIOCap(CONFIG_BT_BLEGC_SECURITY_IO_CAP);
  }

  if (!_adapterRegistry.init()) {
    return false;
  }

  if (!_controllerRegistry.init()) {
    _adapterRegistry.deinit();
    return false;
  }

  if (!_autoScanner.init()) {
    _controllerRegistry.deinit();
    _adapterRegistry.deinit();
    return false;
  }

  BLEGC_LOGD(LOG_TAG, "Initialization completed");

  _initialized = true;
  return true;
}

bool BLEGamepadClient::deinit() {
  if (!_initialized) {
    return false;
  }

  auto result = true;

  result = result && _autoScanner.deinit();
  result = result && _controllerRegistry.deinit();
  result = result && _adapterRegistry.deinit();

  _initialized = false;
  return result;
}

bool BLEGamepadClient::isInitialized() {
  return _initialized;
}

/**
 * @brief Enables the auto-scan feature.
 *
 * Auto-scan automatically starts scanning whenever there are one or more `BLEController` instances that have been
 * initialized with `BLEController::begin()` but are not yet connected. Scanning stops automatically once all
 * `BLEController` instances are connected.
 */

void BLEGamepadClient::enableAutoScan() {
  _autoScanner.enableAutoScan();
}

/**
 * @brief Disables the auto-scan feature.
 *
 * @copydetails enableAutoScan
 */
void BLEGamepadClient::disableAutoScan() {
  _autoScanner.disableAutoScan();
}

/**
 * @brief Checks whether the auto-scan feature is enabled.
 *
 * @copydetails enableAutoScan
 *
 * @return True if auto-scan is enabled; false otherwise.
 */
bool BLEGamepadClient::isAutoScanEnabled() {
  return _autoScanner.isAutoScanEnabled();
}

/**
 * @brief Deletes all stored bonding information.
 */
void BLEGamepadClient::deleteBonds() {
  // TODO
}

/**
 * @brief Registers an adapter for a new controller type. Adapter is used to set up a connection and to
 * decode raw data coming from the controller.
 * @param adapter Adapter to be added.
 * @return True if successful.
 */
bool BLEGamepadClient::addControllerAdapter(const BLEControllerAdapter& adapter) {
  return _adapterRegistry.addAdapter(adapter);
}
