#include "BLEGamepadClient.h"

#include <NimBLEDevice.h>
#include "BLEAutoScanner.h"
#include "BLEDeviceMatcher.h"
#include "BLEControllerRegistry.h"
#include "logger.h"

static auto* LOG_TAG = "BLEGamepadClient";

bool BLEGamepadClient::_initialized(false);
bool BLEGamepadClient::_deleteBonds(false);
TaskHandle_t BLEGamepadClient::_autoScanTask;
BLEDeviceMatcher BLEGamepadClient::_matcher;
BLEControllerRegistry BLEGamepadClient::_controllerRegistry(_autoScanTask, _matcher);
BLEAutoScanner BLEGamepadClient::_autoScanner(_autoScanTask, _controllerRegistry, _matcher);

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

  if (_deleteBonds) {
    NimBLEDevice::deleteAllBonds();
  }

  if (!_matcher.init()) {
    return false;
  }

  if (!_controllerRegistry.init()) {
    _matcher.deinit();
    return false;
  }

  if (!_autoScanner.init()) {
    _controllerRegistry.deinit();
    _matcher.deinit();
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
  result = result && _matcher.deinit();

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
  if (!_initialized) {
    _deleteBonds = true;
    return;
  }
  NimBLEDevice::deleteAllBonds();
}

/**
 * @brief Registers a model for a new controller type. Model is used to set up a connection and to
 * decode raw data coming from the controller.
 * @param model Model to be added.
 * @return True if successful.
 */
bool BLEGamepadClient::addControllerModel(const BLEControllerModel& model) {
  return _matcher.addModel(model);
}
