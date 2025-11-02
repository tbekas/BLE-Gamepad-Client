#include "BLEAutoScanner.h"

#include <NimBLEDevice.h>
#include "BLEControllerRegistry.h"
#include "logger.h"

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
  NimBLEDevice::isInitialized() && xTaskNotifyGive(_autoScanTask);
}

void BLEAutoScanner::disableAutoScan() {
  _autoScanEnabled = false;
  NimBLEDevice::isInitialized() && xTaskNotifyGive(_autoScanTask);
}

bool BLEAutoScanner::isAutoScanEnabled() const {
  return _autoScanEnabled;
}

void BLEAutoScanner::_autoScanTaskFn(void* pvParameters) {
  auto* self = static_cast<BLEAutoScanner*>(pvParameters);

  while (true) {
    ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

    auto* pScan = NimBLEDevice::getScan();
    const auto availConnSlots = self->_controllerRegistry.getAvailableConnectionSlotCount();

    const auto isScanning = pScan->isScanning();
    const auto shouldBeScanning = self->_autoScanEnabled & availConnSlots > 0;

    std::string decisionStr;
    if (!isScanning && shouldBeScanning) {
      decisionStr = "starting scan";
      pScan->start(CONFIG_BT_BLEGC_SCAN_DURATION_MS);
    } else if (isScanning && !shouldBeScanning) {
      decisionStr = "stopping scan";
      pScan->stop();
    } else {
      decisionStr = "no action";
    }

    BLEGC_LOGD("Auto-scan %s, %sscan in progress, available connection slots: %d -> %s",
        self->_autoScanEnabled ? "enabled" : "disabled",
        isScanning ? "" : "no ",
        availConnSlots,
        decisionStr.c_str());
  }
}

BLEAutoScanner::ScanCallbacks::ScanCallbacks(BLEAutoScanner& autoScanner) : _autoScanner(autoScanner) {}

void BLEAutoScanner::ScanCallbacks::onResult(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  BLEGC_LOGD("Device discovered, address: %s, address type: %d, name: %s",
             std::string(pAdvertisedDevice->getAddress()).c_str(), pAdvertisedDevice->getAddressType(),
             pAdvertisedDevice->getName().c_str());

  _autoScanner._controllerRegistry.tryConnectController(pAdvertisedDevice);
}

void BLEAutoScanner::ScanCallbacks::onScanEnd(const NimBLEScanResults& results, int reason) {
  BLEGC_LOGD("Scan ended, reason: 0x%04x %s", reason, NimBLEUtils::returnCodeToString(reason));
  xTaskNotifyGive(_autoScanner._autoScanTask);
}
