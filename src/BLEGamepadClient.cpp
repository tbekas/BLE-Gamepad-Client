#include "BLEGamepadClient.h"

#include <NimBLEDevice.h>
#include "BLEAutoScan.h"
#include "BLEControllerRegistry.h"
#include "BLEUserCallbackRunner.h"
#include "logger.h"
#include "config.h"

bool BLEGamepadClient::_initialized = false;
TaskHandle_t BLEGamepadClient::_autoScanTask;
QueueHandle_t BLEGamepadClient::_userCallbackQueue;
BLEControllerRegistry BLEGamepadClient::_controllerRegistry(_autoScanTask, _userCallbackQueue);
BLEAutoScan BLEGamepadClient::_autoScan(_controllerRegistry, _autoScanTask, _userCallbackQueue);
BLEUserCallbackRunner BLEGamepadClient::_userCallbackRunner(_autoScan, _userCallbackQueue);

/**
 * @brief Initializes the library.
 *
 * This method is called automatically; there is no need to invoke it explicitly in your application code.
 *
 * @param deleteBonds If true, deletes all stored bonding information.
 */
void BLEGamepadClient::init(const bool deleteBonds) {
  _initSelf();

  if (!NimBLEDevice::isInitialized()) {
    BLEGC_LOGD("Initializing NimBLE");
    NimBLEDevice::init(CONFIG_BT_BLEGC_DEVICE_NAME);
    NimBLEDevice::setPower(CONFIG_BT_BLEGC_POWER_DBM);
    NimBLEDevice::setSecurityAuth(CONFIG_BT_BLEGC_SECURITY_AUTH);
    NimBLEDevice::setSecurityIOCap(CONFIG_BT_BLEGC_SECURITY_IO_CAP);

    if (deleteBonds) {
      NimBLEDevice::deleteAllBonds();
    }
  }
}

/**
 * @brief Enables debug-level logging.
 */
void BLEGamepadClient::enableDebugLog() {
  _initSelf();

  blegc::setLogLevelDebug();
}

BLEAutoScan* BLEGamepadClient::getAutoScan() {
  _initSelf();

  return &_autoScan;
}

void BLEGamepadClient::_initSelf() {
  if (!_initialized) {
    blegc::setDefaultLogLevel();

    _initialized = true;
  }
}
