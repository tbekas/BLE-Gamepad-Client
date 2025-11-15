#include "BLEAutoScan.h"

#include <NimBLEDevice.h>
#include "BLEControllerRegistry.h"
#include "logger.h"

BLEAutoScan::BLEAutoScan(TaskHandle_t& startStopScanTask,
                         TaskHandle_t& scanCallbackTask,
                         BLEControllerRegistry& controllerRegistry)
    : _startStopScanTask(startStopScanTask),
      _scanCallbackTask(scanCallbackTask),
      _restartCount(0),
      _controllerRegistry(controllerRegistry),
      _bleScanCallbacks(*this),
      _onScanStart([]() {}),
      _onScanStop([]() {}) {
  xTaskCreate(_startStopScanTaskFn, "_startStopScanTaskFn", 10000, this, 0, &_startStopScanTask);
  configASSERT(_startStopScanTask);

  xTaskCreate(_scanCallbackTaskFn, "_scanCallbackTaskFn", 10000, this, 0, &_scanCallbackTask);
  configASSERT(_scanCallbackTask);

  auto* pScan = NimBLEDevice::getScan();
  pScan->setScanCallbacks(&_bleScanCallbacks, false);
  pScan->setMaxResults(0);
}

/**
 * @brief Enables the auto-scan feature (enabled by default).
 *
 * Auto-scan automatically starts scanning whenever there are one or more controller instances that have been
 * initialized with `begin()` but are not yet connected. Scanning stops automatically once all controller instances
 * are connected.
 */
void BLEAutoScan::enable() {
  _enabled = true;
  notify();
}

/**
 * @brief Disables the auto-scan feature.
 *
 * @copydetails enable
 */
void BLEAutoScan::disable() {
  _enabled = false;
  notify();
}

/**
 * @brief Checks whether the auto-scan feature is enabled.
 *
 * @copydetails enable
 *
 * @return True if auto-scan is enabled; false otherwise.
 */
bool BLEAutoScan::isEnabled() const {
  return _enabled;
}
bool BLEAutoScan::isScanning() const {
  return NimBLEDevice::isInitialized() && NimBLEDevice::getScan()->isScanning();
}

/**
 * @brief Triggers auto-scan check. If previous scan ended without filling all available connection slots, you can
 * start another scan by calling this method.
 *
 * @copydetails enable
 */
void BLEAutoScan::notify() const {
  NimBLEDevice::isInitialized() && xTaskNotify(_startStopScanTask, 0, eSetValueWithOverwrite);
}

void BLEAutoScan::onScanStart(const std::function<void()>& callback) {
  _onScanStart = callback;
}
void BLEAutoScan::onScanStop(const std::function<void()>& callback) {
  _onScanStop = callback;
}

void BLEAutoScan::_scanCallbackTaskFn(void* pvParameters) {
  auto* self = static_cast<BLEAutoScan*>(pvParameters);

  while (true) {
    const auto val = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    const auto scanStarted = static_cast<bool>(val);

    if (self->_enabled) {
      if (scanStarted) {
        self->_onScanStart();
      } else {
        self->_onScanStop();
      }
    }
  }
}

void BLEAutoScan::_startStopScanTaskFn(void* pvParameters) {
  auto* self = static_cast<BLEAutoScan*>(pvParameters);

  while (true) {
    const auto val = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // Indicates whether this notification follows a previous scan completion
    const auto isRestart = static_cast<bool>(val);

    auto* pScan = NimBLEDevice::getScan();
    const auto availConnSlots = self->_controllerRegistry.getAvailableConnectionSlotCount();

    const auto isScanning = pScan->isScanning();
    const auto shouldBeScanning = self->_enabled & availConnSlots > 0;

    std::string decisionStr;
    if (!isScanning && shouldBeScanning) {
      if (isRestart) {
        if (self->_restartCount < CONFIG_BT_BLEGC_LOW_DUTY_SCAN_RESTART_LIMIT) {
          decisionStr = "start low duty scan";
          pScan->setWindow(CONFIG_BT_BLEGC_LOW_DUTY_SCAN_WINDOW_MS);
          pScan->setInterval(CONFIG_BT_BLEGC_LOW_DUTY_SCAN_INTERVAL_MS);
          pScan->setActiveScan(CONFIG_BT_BLEGC_LOW_DUTY_SCAN_ACTIVE > 0);
          pScan->start(CONFIG_BT_BLEGC_LOW_DUTY_SCAN_DURATION_MS);
          self->_restartCount++;
        } else {
          decisionStr = "scan start limit reached";
          xTaskNotify(self->_scanCallbackTask, 0, eSetValueWithOverwrite);
        }
      } else {
        decisionStr = "start high duty scan";
        pScan->setWindow(CONFIG_BT_BLEGC_HIGH_DUTY_SCAN_WINDOW_MS);
        pScan->setInterval(CONFIG_BT_BLEGC_HIGH_DUTY_SCAN_INTERVAL_MS);
        pScan->setActiveScan(CONFIG_BT_BLEGC_HIGH_DUTY_SCAN_ACTIVE > 0);
        pScan->start(CONFIG_BT_BLEGC_HIGH_DUTY_SCAN_DURATION_MS);

        self->_restartCount = 0;
        xTaskNotify(self->_scanCallbackTask, 1, eSetValueWithOverwrite);
      }
    } else if (isScanning && !shouldBeScanning) {
      decisionStr = "stop scan";
      pScan->stop();
      xTaskNotify(self->_scanCallbackTask, 0, eSetValueWithOverwrite);
    } else {
      decisionStr = "do nothing";
    }

    BLEGC_LOGD("Auto-scan enabled: %d, scan in progress: %d, avail connection slots: %d, restart count: %d -> %s",
               self->_enabled, isScanning, availConnSlots, self->_restartCount, decisionStr.c_str());
  }
}

BLEAutoScan::ScanCallbacks::ScanCallbacks(BLEAutoScan& autoScan) : _autoScan(autoScan) {}

void BLEAutoScan::ScanCallbacks::onResult(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  BLEGC_LOGD("Device discovered, address: %s, address type: %d, name: %s",
             std::string(pAdvertisedDevice->getAddress()).c_str(), pAdvertisedDevice->getAddressType(),
             pAdvertisedDevice->getName().c_str());

  _autoScan._controllerRegistry.tryConnectController(pAdvertisedDevice);
}

void BLEAutoScan::ScanCallbacks::onScanEnd(const NimBLEScanResults& results, int reason) {
  BLEGC_LOGD("Scan ended, reason: 0x%04x %s", reason, NimBLEUtils::returnCodeToString(reason));
  xTaskNotify(_autoScan._startStopScanTask, 1, eSetValueWithOverwrite);
}
