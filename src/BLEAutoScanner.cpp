#include "BLEAutoScanner.h"

#include <NimBLEDevice.h>
#include "BLEControllerRegistry.h"
#include "logger.h"

static auto* LOG_TAG = "BLEAutoScanner";

BLEAutoScanner::BLEAutoScanner(TaskHandle_t& autoScanTask,
                               BLEControllerRegistry& controllerRegistry)
    : _autoScanTask(autoScanTask),
      _controllerRegistry(controllerRegistry),
      _scanCallbacks(*this) {
  xTaskCreate(_autoScanTaskFn, "_autoScanTaskFn", 10000, this, 0, &_autoScanTask);
  configASSERT(_autoScanTask);

  auto* pScan = NimBLEDevice::getScan();
  pScan->setScanCallbacks(&_scanCallbacks, false);
  pScan->setActiveScan(true);
  pScan->setMaxResults(0);
}

void BLEAutoScanner::enableAutoScan() {
  _autoScanEnabled = true;
}

void BLEAutoScanner::disableAutoScan() {
  _autoScanEnabled = false;
}

bool BLEAutoScanner::isAutoScanEnabled() const {
  return _autoScanEnabled;
}

void BLEAutoScanner::_autoScanTaskFn(void* pvParameters) {
  auto* self = static_cast<BLEAutoScanner*>(pvParameters);

  while (true) {
    ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

    auto* pScan = NimBLEDevice::getScan();
    const auto isScanning = pScan->isScanning();
    const auto isEnabled = self->_autoScanEnabled;

    if (isEnabled && isScanning) {
      BLEGC_LOGD(LOG_TAG, "Auto-scan enabled, scan already in-progress");
      // do nothing
    } else if (isEnabled && !isScanning) {
      if (self->_controllerRegistry.getAvailableConnectionSlotCount() == 0) {
        BLEGC_LOGD(LOG_TAG, "Auto-scan enabled, no scan in-progress, no available connection slots left");
      } else {
        BLEGC_LOGD(LOG_TAG, "Auto-scan enabled, no scan in-progress, connection slots available -> starting scan");
        pScan->start(CONFIG_BT_BLEGC_SCAN_DURATION_MS);
      }
    } else if (!isEnabled && isScanning) {
      BLEGC_LOGD(LOG_TAG, "Auto-scan disabled, scan in-progress -> stopping scan");
      pScan->stop();
    } else if (!isEnabled && !isScanning) {
      BLEGC_LOGD(LOG_TAG, "Auto-scan disabled, no scan in-progress");
      // do nothing
    }
  }
}

BLEAutoScanner::ScanCallbacks::ScanCallbacks(BLEAutoScanner& autoScanner) : _autoScanner(autoScanner) {}

void BLEAutoScanner::ScanCallbacks::onResult(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  BLEGC_LOGD(LOG_TAG, "Device discovered, address: %s, address type: %d, name: %s",
             std::string(pAdvertisedDevice->getAddress()).c_str(), pAdvertisedDevice->getAddressType(),
             pAdvertisedDevice->getName().c_str());

  _autoScanner._controllerRegistry.tryConnectController(pAdvertisedDevice);
}

void BLEAutoScanner::ScanCallbacks::onScanEnd(const NimBLEScanResults& results, int reason) {
  BLEGC_LOGD(LOG_TAG, "Scan ended, reason: 0x%04x %s", reason, NimBLEUtils::returnCodeToString(reason));
  xTaskNotifyGive(_autoScanner._autoScanTask);
}
