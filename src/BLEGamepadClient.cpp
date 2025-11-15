#include "BLEGamepadClient.h"

#include <NimBLEDevice.h>
#include "BLEAutoScan.h"
#include "BLEControllerRegistry.h"
#include "logger.h"

bool BLEGamepadClient::_initialized = false;
TaskHandle_t BLEGamepadClient::_startStopScanTask;
TaskHandle_t BLEGamepadClient::_scanCallbackTask;
BLEControllerRegistry BLEGamepadClient::_controllerRegistry(_startStopScanTask, _scanCallbackTask);
BLEAutoScan BLEGamepadClient::_autoScan(_startStopScanTask, _scanCallbackTask, _controllerRegistry);

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

BLEAutoScan* BLEGamepadClient::getAutoScan() {
  init(false);

  return &_autoScan;
}
