#include "BLEGamepadClient.h"

#include <NimBLEDevice.h>
#include "BLEAutoScanner.h"
#include "BLEControllerRegistry.h"
#include "logger.h"

static auto* LOG_TAG = "BLEGamepadClient";

TaskHandle_t BLEGamepadClient::_autoScanTask;
BLEControllerRegistry BLEGamepadClient::_controllerRegistry(_autoScanTask);
BLEAutoScanner BLEGamepadClient::_autoScanner(_autoScanTask, _controllerRegistry);

void BLEGamepadClient::initBLEDevice() {
  if (!NimBLEDevice::isInitialized()) {
    BLEGC_LOGD(LOG_TAG, "Initializing NimBLE");
    NimBLEDevice::init(CONFIG_BT_BLEGC_DEVICE_NAME);
    NimBLEDevice::setPower(CONFIG_BT_BLEGC_POWER_DBM);
    NimBLEDevice::setSecurityAuth(CONFIG_BT_BLEGC_SECURITY_AUTH);
    NimBLEDevice::setSecurityIOCap(CONFIG_BT_BLEGC_SECURITY_IO_CAP);
  }
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
  initBLEDevice();

  NimBLEDevice::deleteAllBonds();
}
