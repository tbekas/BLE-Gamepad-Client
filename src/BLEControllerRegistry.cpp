#include "BLEControllerRegistry.h"

#include <NimBLEClient.h>
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEUtils.h>
#include <bitset>
#include "BLEClientCallbacksImpl.h"
#include "BLEScanCallbacksImpl.h"
#include "config.h"
#include "logger.h"
#include "xbox.h"

BLEScanCallbacksImpl scanCallbacks;

BLEClientStatus::operator std::string() const {
  auto kindStr = kind == BLEClientConnected ? "BLEClientConnected" : "BLEClientDisconnected";
  return "BLEClientStatus address: " + std::string(address) + ", kind: " + kindStr;
}

/**
 * @brief Initializes a BLEGamepadClient.
 * @return True if successful.
 */
bool BLEControllerRegistry::init() {
  if (_initialized) {
    return false;
  }

  _connectionSlots = xSemaphoreCreateCounting(CONFIG_BT_NIMBLE_MAX_CONNECTIONS, 0);
  configASSERT(_connectionSlots);
  _clientStatusQueue = xQueueCreate(10, sizeof(BLEClientStatus));
  configASSERT(_clientStatusQueue);

  xTaskCreatePinnedToCore(_clientStatusConsumerFn, "_bleConnectionMsgConsumerTask", 10000, nullptr, 0,
                          &_clientStatusConsumerTask, CONFIG_BT_NIMBLE_PINNED_TO_CORE);
  configASSERT(_clientStatusConsumerTask);

  // default config - lowest priority
  _configs.push_front(xbox::controllerConfig);

  if (!NimBLEDevice::isInitialized()) {
    NimBLEDevice::init(CONFIG_BT_BLEGC_DEVICE_NAME);
    NimBLEDevice::setPower(CONFIG_BT_BLEGC_POWER_DBM);
    NimBLEDevice::setSecurityAuth(CONFIG_BT_BLEGC_SECURITY_AUTH);
    NimBLEDevice::setSecurityIOCap(CONFIG_BT_BLEGC_SECURITY_IO_CAP);
  }

  auto* pScan = NimBLEDevice::getScan();
  pScan->setScanCallbacks(&scanCallbacks, false);
  pScan->setActiveScan(true);
  pScan->setMaxResults(0);

  _initialized = true;
  return true;
}

/**
 * @brief Deinitializes a BLEGamepadClient.
 * @return True if successful.
 */
bool BLEControllerRegistry::deinit() {
  if (!_initialized) {
    return false;
  }

  bool result = true;

  auto* pScan = NimBLEDevice::getScan();
  if (pScan->isScanning()) {
    result = pScan->stop() && result;
  }

  for (auto& ctrl : _controllers) {
    if (ctrl.isInitialized()) {
      result = ctrl.deinit(false) && result;
    }

    auto* pClient = BLEDevice::getClientByPeerAddress(ctrl.getAddress());
    if (!pClient) {
      continue;
    }
    pClient->disconnect();
  }

  _controllers.clear();

  result = NimBLEDevice::deinit() && result;

  if (_clientStatusConsumerTask != nullptr) {
    vTaskDelete(_clientStatusConsumerTask);
  }
  if (_clientStatusQueue != nullptr) {
    vQueueDelete(_clientStatusQueue);
  }
  if (_connectionSlots != nullptr) {
    vSemaphoreDelete(_connectionSlots);
  }

  _initialized = false;
  return result;
}

/**
 * @brief Checks if BLEGamepadClient is initialized.
 * @return True if initialized; false otherwise.
 */
bool BLEControllerRegistry::isInitialized() {
  return _initialized;
}

/**
 * @brief Enables the auto-scan feature.
 *
 * Auto-scan automatically starts scanning whenever there are one or more `BLEController` instances that have been
 * initialized with `BLEController::begin()` but are not yet connected. Scanning stops automatically once all `BLEController`
 * instances are connected.
 */
void BLEControllerRegistry::enableAutoScan() {
  _autoScanEnabled = true;
  if (_initialized) {
    _autoScanCheck();
  }
}

/**
 * @brief Disables the auto-scan feature.
 *
 * @copydetails enableAutoScan
 */
void BLEControllerRegistry::disableAutoScan() {
  _autoScanEnabled = false;
  if (_initialized) {
    _autoScanCheck();
  }
}

/**
 * @brief Checks whether the auto-scan feature is enabled.
 *
 * @copydetails enableAutoScan
 *
 * @return True if auto-scan is enabled; false otherwise.
 */
bool BLEControllerRegistry::isAutoScanEnabled() {
  return _autoScanEnabled;
}
/**
 * @brief Registers a configuration for a new controller type. The configuration is used to set up a connection and to
 * decode raw data coming from the controller.
 * @param config Configuration to be registered.
 * @return True if successful.
 */
bool BLEControllerRegistry::addControllerConfig(const BLEControllerAdapter& config) {
  if (_configs.size() >= MAX_CTRL_CONFIG_COUNT) {
    BLEGC_LOGE("Reached maximum number of configs: %d", MAX_CTRL_CONFIG_COUNT);
    return false;
  }
  if (config.controls.isDisabled() && config.battery.isDisabled() && config.vibrations.isDisabled()) {
    BLEGC_LOGE("Invalid config, at least one of [`controls`, `battery`, `vibrations`] has to be enabled");
    return false;
  }

  _configs.push_back(config);
  return true;
}

BLEControllerInternal* BLEControllerRegistry::_createController(NimBLEAddress allowedAddress) {
  if (xSemaphoreGive(_connectionSlots) != pdTRUE) {
    BLEGC_LOGE("Cannot create controller");
    return nullptr;
  }

  BLEGC_LOGD("Creating a new controller instance, address: %s", std::string(allowedAddress).c_str());
  _controllers.emplace_back(allowedAddress);
  auto& ctrl = _controllers.back();

  _autoScanCheck();
  return &ctrl;
}

BLEControllerInternal* BLEControllerRegistry::_getController(NimBLEAddress address) {
  for (auto& ctrl : _controllers) {
    if (ctrl.getAddress() == address) {
      return &ctrl;
    }
  }

  return nullptr;
}

bool BLEControllerRegistry::_reserveController(const NimBLEAddress address) {
  if (xSemaphoreTake(_connectionSlots, 0) != pdTRUE) {
    BLEGC_LOGD("No connections slots left");
    return false;
  }

  // try reserve ctrl with matching allowed address
  for (auto& ctrl : _controllers) {
    if (!ctrl.getAddress().isNull()) {
      continue;
    }

    if (!ctrl.getAllowedAddress().isNull() && ctrl.getAllowedAddress() == address) {
      ctrl.setAddress(address);
      return true;
    }
  }

  // try reserve ctrl reserved last time
  for (auto& ctrl : _controllers) {
    if (!ctrl.getAddress().isNull()) {
      continue;
    }

    if (!ctrl.getLastAddress().isNull() && ctrl.getLastAddress() == address) {
      ctrl.setAddress(address);
      return true;
    }
  }

  // try reserve any controller
  for (auto& ctrl : _controllers) {
    if (!ctrl.getAddress().isNull()) {
      continue;
    }

    if (ctrl.getAllowedAddress().isNull()) {
      ctrl.setAddress(address);
      return true;
    }
  }

  BLEGC_LOGD("No suitable controller available");

  if (xSemaphoreGive(_connectionSlots) != pdTRUE) {
    BLEGC_LOGE("Failed to release connection slot");
    return false;
  }

  return false;
}

bool BLEControllerRegistry::_releaseController(const NimBLEAddress address) {
  auto* pCtrl = _getController(address);
  if (pCtrl == nullptr) {
    BLEGC_LOGE("Controller not found, address: %s", std::string(address).c_str());
    return false;
  }

  pCtrl->setLastAddress(pCtrl->getAddress());
  pCtrl->setAddress(NimBLEAddress());

  if (xSemaphoreGive(_connectionSlots) != pdTRUE) {
    BLEGC_LOGE("Failed to release connection slot");
    return false;
  }
  return true;
}

void BLEControllerRegistry::_clientStatusConsumerFn(void* pvParameters) {
  while (true) {
    BLEClientStatus msg{};
    if (xQueueReceive(_clientStatusQueue, &msg, portMAX_DELAY) != pdTRUE) {
      BLEGC_LOGE("Failed to receive client status message");
      return;
    }

    BLEGC_LOGD("Handling client status message %s", std::string(msg).c_str());

    switch (msg.kind) {
      case BLEClientConnected: {
        auto* pCtrl = _getController(msg.address);
        if (pCtrl == nullptr) {
          BLEGC_LOGE("Controller not found, address: %s", std::string(msg.address).c_str());
          break;
        }

        auto configMatch = std::bitset<MAX_CTRL_CONFIG_COUNT>(_configMatch[msg.address]);
        const int configsSize = _configs.size();
        // reverse iteration order
        for (int i = configsSize - 1; i >= 0; i--) {
          if (!configMatch[i]) {
            continue;
          }

          auto& config = _configs[i];

          if (!pCtrl->init(config)) {
            BLEGC_LOGW("Failed to initialize controller, address: %s", std::string(msg.address).c_str());
            // this config failed, try another one
            continue;
          }

          BLEGC_LOGD("Controller sucesfuly initialized");
          break;
        }
        // TODO: disconnect and tmp ban?
      } break;
      case BLEClientDisconnected:
        auto* pCtrl = _getController(msg.address);
        if (pCtrl == nullptr) {
          BLEGC_LOGE("Controller not found, address: %s", std::string(msg.address).c_str());
          break;
        }

        if (pCtrl->isInitialized()) {
          if (!pCtrl->deinit(true)) {
            BLEGC_LOGW("Controller failed to deinitialize, address: %s", std::string(msg.address).c_str());
          }
          BLEGC_LOGD("Controller sucesfuly deinitialized");
        }

        _releaseController(msg.address);
        _autoScanCheck();
        break;
    }
  }
}

void BLEControllerRegistry::_autoScanCheck() {
  auto* pScan = NimBLEDevice::getScan();
  const auto isScanning = pScan->isScanning();

  if (_autoScanEnabled && isScanning) {
    BLEGC_LOGD("Auto-scan enabled, scan in progress");
    // do nothing
  } else if (_autoScanEnabled && !isScanning) {
    if (uxSemaphoreGetCount(_connectionSlots) == 0) {
      BLEGC_LOGD("Auto-scan enabled, no available connection slots");
    } else {
      BLEGC_LOGD("Auto-scan enabled, starting scan");
      pScan->start(CONFIG_BT_BLEGC_SCAN_DURATION_MS);
    }
  } else if (!_autoScanEnabled && isScanning) {
    BLEGC_LOGD("Auto-scan disabled, stopping scan");
    pScan->stop();
  } else if (!_autoScanEnabled && !isScanning) {
    BLEGC_LOGD("Auto-scan disabled, no scan in-progress");
    // do nothing
  }
}

bool BLEControllerRegistry::_initialized{false};
bool BLEControllerRegistry::_autoScanEnabled{true};
QueueHandle_t BLEControllerRegistry::_clientStatusQueue{nullptr};
TaskHandle_t BLEControllerRegistry::_clientStatusConsumerTask{nullptr};
SemaphoreHandle_t BLEControllerRegistry::_connectionSlots{nullptr};
std::map<NimBLEAddress, uint64_t> BLEControllerRegistry::_configMatch{};
std::list<BLEControllerInternal> BLEControllerRegistry::_controllers{};
std::deque<BLEControllerAdapter> BLEControllerRegistry::_configs{};
