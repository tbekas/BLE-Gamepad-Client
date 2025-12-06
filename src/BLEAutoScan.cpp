#include "BLEAutoScan.h"

#include <NimBLEDevice.h>
#include "BLEControllerRegistry.h"
#include "messages.h"
#include "logger.h"

BLEAutoScan::BLEAutoScan(BLEControllerRegistry& controllerRegistry,
                         TaskHandle_t& startStopScanTask,
                         QueueHandle_t& userCallbackQueue)
    : _startStopScanTask(startStopScanTask),
      _startTimeMs(0),
      _controllerRegistry(controllerRegistry),
      _scanCallbacksImpl(*this),
      _onScanStarted([]() {}),
      _onScanStopped([]() {}),
      _userCallbackQueue(userCallbackQueue) {
  xTaskCreate(_startStopScanTaskFn, "_startStopScanTaskFn", 10000, this, 0, &_startStopScanTask);
  configASSERT(_startStopScanTask);

  auto* pScan = NimBLEDevice::getScan();
  pScan->setScanCallbacks(&_scanCallbacksImpl, false);
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
  NimBLEDevice::isInitialized() &&
      xTaskNotify(_startStopScanTask, static_cast<uint8_t>(BLEAutoScanNotification::Auto), eSetValueWithOverwrite);
}

void BLEAutoScan::onScanStarted(const std::function<void()>& callback) {
  _onScanStarted = callback;
}

void BLEAutoScan::onScanStopped(const std::function<void()>& callback) {
  _onScanStopped = callback;
}

void BLEAutoScan::callOnScanStarted() {
  _onScanStarted();
}

void BLEAutoScan::callOnScanStopped() {
  _onScanStopped();
}

void BLEAutoScan::_sendUserCallbackMsg(const BLEUserCallback& msg) const {
  if (xQueueSend(_userCallbackQueue, &msg, 0) != pdPASS) {
    BLEGC_LOGE("Failed to send user callback message");
  }
}

void BLEAutoScan::_startScan(NimBLEScan* pScan, bool highDuty) {
  if (highDuty) {
    pScan->setWindow(CONFIG_BT_BLEGC_HIGH_DUTY_SCAN_WINDOW_MS);
    pScan->setInterval(CONFIG_BT_BLEGC_HIGH_DUTY_SCAN_INTERVAL_MS);
    pScan->setActiveScan(CONFIG_BT_BLEGC_HIGH_DUTY_SCAN_ACTIVE > 0);
    pScan->start(CONFIG_BT_BLEGC_HIGH_DUTY_SCAN_DURATION_MS);
  } else {
    pScan->setWindow(CONFIG_BT_BLEGC_LOW_DUTY_SCAN_WINDOW_MS);
    pScan->setInterval(CONFIG_BT_BLEGC_LOW_DUTY_SCAN_INTERVAL_MS);
    pScan->setActiveScan(CONFIG_BT_BLEGC_LOW_DUTY_SCAN_ACTIVE > 0);
    pScan->start(CONFIG_BT_BLEGC_LOW_DUTY_SCAN_DURATION_MS);
  }
  _sendUserCallbackMsg({BLEUserCallbackKind::BLEScanStarted});
}

void BLEAutoScan::_stopScan(NimBLEScan* pScan) {
  pScan->stop();
  _sendUserCallbackMsg({BLEUserCallbackKind::BLEScanStopped});
}

void BLEAutoScan::_startStopScanTaskFn(void* pvParameters) {
  auto* self = static_cast<BLEAutoScan*>(pvParameters);

  while (true) {
    const auto val = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    auto* pScan = NimBLEDevice::getScan();
    const auto allocInfo = self->_controllerRegistry.getControllerAllocationInfo();
    const auto canAllocateCtrl = allocInfo.notAllocated > 0 && allocInfo.allocated < CONFIG_BT_NIMBLE_MAX_CONNECTIONS;
    const auto isScanning = pScan->isScanning();
    const auto currTimeMs = millis();
    std::string decision;

    switch (static_cast<BLEAutoScanNotification>(val)) {
      case BLEAutoScanNotification::ScanFinished: {
        if (self->_enabled && canAllocateCtrl) {

          if (!isScanning) {
            const auto hdEndTimeMs = self->_startTimeMs + CONFIG_BT_BLEGC_HIGH_DUTY_SCAN_DURATION_MS;
            const auto ldEndTimeMs = hdEndTimeMs + CONFIG_BT_BLEGC_LOW_DUTY_SCAN_DURATION_MS;

            if (currTimeMs > hdEndTimeMs && currTimeMs < ldEndTimeMs) {
              decision = "start low duty scan";
              self->_startScan(pScan, false);
            } else {
              decision = "do nothing (timeout)";
            }
          } else {
            decision = "do nothing";
          }
        }
        break;
      }
      case BLEAutoScanNotification::Auto: {
        if (self->_enabled && canAllocateCtrl) {
          decision = "start high duty scan";
          self->_startTimeMs = currTimeMs;
          self->_startScan(pScan, true);
        } else {
          if (isScanning) {
            self->_stopScan(pScan);
            decision = "stop scan";
          } else {
            decision = "do nothing";
          }
        }
        break;
      }
    }

    BLEGC_LOGD("AutoScan is enabled: %d, is scanning: %d, allocated ctrls: %d/%d, decision: %s",
               self->_enabled, isScanning, allocInfo.allocated, allocInfo.allocated + allocInfo.notAllocated,
               decision.c_str());
  }
}

BLEAutoScan::ScanCallbacksImpl::ScanCallbacksImpl(BLEAutoScan& autoScan) : _autoScan(autoScan) {}

void BLEAutoScan::ScanCallbacksImpl::onResult(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  BLEGC_LOGD("Device discovered, address: %s, address type: %d, name: %s",
             std::string(pAdvertisedDevice->getAddress()).c_str(), pAdvertisedDevice->getAddressType(),
             pAdvertisedDevice->getName().c_str());

  _autoScan._controllerRegistry.tryConnectController(pAdvertisedDevice);
}

void BLEAutoScan::ScanCallbacksImpl::onScanEnd(const NimBLEScanResults& results, int reason) {
  BLEGC_LOGD("Scan ended, reason: 0x%04x %s", reason, NimBLEUtils::returnCodeToString(reason));
  xTaskNotify(_autoScan._startStopScanTask, static_cast<uint8_t>(BLEAutoScanNotification::ScanFinished), eSetValueWithOverwrite);
}
