#include "BLEAutoScan.h"

static auto* LOG_TAG = "BLEAutoScan";

BLEAutoScan::BLEAutoScan(BLEScanCallbacksImpl& scanCallbacks) : _scanCallbacks(scanCallbacks) {}

void BLEAutoScan::enable() {
  _enabled = true;
}

void BLEAutoScan::disable() {
  _enabled = false;
}

bool BLEAutoScan::isEnabled() const {
  return _enabled;
}
bool BLEAutoScan::init() {
  auto* pScan = NimBLEDevice::getScan();
  pScan->setScanCallbacks(&_scanCallbacks, false);
  pScan->setActiveScan(true);
  pScan->setMaxResults(0);
}
bool BLEAutoScan::isInitialized() const {
  return _initialized;
}
void BLEAutoScan::trigger() {
  auto* pScan = NimBLEDevice::getScan();
  const auto isScanning = pScan->isScanning();

  if (_enabled && isScanning) {
    BLEGC_LOGD(LOG_TAG, "Auto-scan enabled, scan already in-progress");
    // do nothing
  } else if (_enabled && !isScanning) {
    if (uxSemaphoreGetCount(_connectionSlots) == 0) {
      BLEGC_LOGD(LOG_TAG, "Auto-scan enabled, no scan in-progress, no available connection slots left");
    } else {
      BLEGC_LOGD(LOG_TAG, "Auto-scan enabled, no scan in-progress, connection slots available -> starting scan");
      pScan->start(CONFIG_BT_BLEGC_SCAN_DURATION_MS);
    }
  } else if (!_enabled && isScanning) {
    BLEGC_LOGD(LOG_TAG, "Auto-scan disabled, scan in-progress -> stopping scan");
    pScan->stop();
  } else if (!_enabled && !isScanning) {
    BLEGC_LOGD(LOG_TAG, "Auto-scan disabled, no scan in-progress");
    // do nothing
  }
}
