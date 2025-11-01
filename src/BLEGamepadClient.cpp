#include "BLEGamepadClient.h"

#include <NimBLEDevice.h>
#include "BLEAutoScanner.h"
#include "BLEControllerRegistry.h"
#include "logger.h"

bool BLEGamepadClient::_initialized = false;
TaskHandle_t BLEGamepadClient::_autoScanTask;
BLEControllerRegistry BLEGamepadClient::_controllerRegistry(_autoScanTask);
BLEAutoScanner BLEGamepadClient::_autoScanner(_autoScanTask, _controllerRegistry);

/**
 * @brief Initializes the library.
 *
 * This method is called automatically, so there is no need to invoke it explicitly in your application code.
 *
 * @param initBLE Specifies whether to initialize the NimBLE library.
 */
void BLEGamepadClient::init(const bool initBLE) {
  if (!_initialized) {
    blegc::setDefaultLogLevel();

    _initialized = true;
  }

  if (initBLE && !NimBLEDevice::isInitialized()) {
    BLEGC_LOGD("Initializing NimBLE");
    NimBLEDevice::init(CONFIG_BT_BLEGC_DEVICE_NAME);
    NimBLEDevice::setPower(CONFIG_BT_BLEGC_POWER_DBM);
    NimBLEDevice::setSecurityAuth(CONFIG_BT_BLEGC_SECURITY_AUTH);
    NimBLEDevice::setSecurityIOCap(CONFIG_BT_BLEGC_SECURITY_IO_CAP);
  }
}

/**
 * @brief Enables the auto-scan feature.
 *
 * Auto-scan automatically starts scanning whenever there are one or more controller instances that have been
 * initialized with `begin()` but are not yet connected. Scanning stops automatically once all controller instances
 * are connected.
 */
void BLEGamepadClient::enableAutoScan() {
  init(false);

  _autoScanner.enableAutoScan();
}

/**
 * @brief Disables the auto-scan feature.
 *
 * @copydetails enableAutoScan
 */
void BLEGamepadClient::disableAutoScan() {
  init(false);

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
  init(false);

  return _autoScanner.isAutoScanEnabled();
}

/**
 * @brief Deletes all stored bonding information.
 */
void BLEGamepadClient::deleteBonds() {
  init();

  NimBLEDevice::deleteAllBonds();
}

/**
 * @brief Enables debug-level logging.
 */
void BLEGamepadClient::enableDebugLog() {
  init(false);

  blegc::setLogLevelDebug();
}
