#include "BLEGamepadClient.h"

#include <NimBLEClient.h>
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEUtils.h>
#include <bitset>
#include "ClientCallbacks.h"
#include "ScanCallbacks.h"
#include "config.h"
#include "logger.h"
#include "xbox.h"

ScanCallbacks scanCallbacks;

BLEClientStatus::operator std::string() const {
  auto kindStr = kind == BLEClientConnected ? "BLEClientConnected" : "BLEClientDisconnected";
  return "BLEClientStatus address: " + std::string(address) + ", kind: " + kindStr;
}

bool BLEGamepadClient::init() {
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

  if constexpr (CONFIG_BT_BLEGC_DELETE_BONDS) {
    NimBLEDevice::deleteAllBonds();
  }

  NimBLEScan* pScan = NimBLEDevice::getScan();
  pScan->setScanCallbacks(&scanCallbacks, false);
  pScan->setActiveScan(true);
  pScan->setMaxResults(0);
  _autoScanCheck();

  _initialized = true;
  return true;
}

/**
 * @brief Deinitializes a GamepadClient instance.
 * @return True if successful.
 */
bool BLEGamepadClient::deinit() {
  if (!_initialized) {
    return false;
  }

  bool result = true;

  NimBLEScan* pScan = NimBLEDevice::getScan();
  if (pScan->isScanning()) {
    result = pScan->stop() && result;
  }

  for (auto& ctrl : _controllers) {
    if (ctrl.isInitialized()) {
      result = ctrl.deinit(false) && result;
    }

    auto pClient = BLEDevice::getClientByPeerAddress(ctrl.getAddress());
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

bool BLEGamepadClient::isInitialized() {
  return _initialized;
}

/**
 * @brief Registers a configuration for a new controller type. The configuration is used to set up a connection and to
 * decode raw data coming from the controller.
 * @param config Configuration to be registered.
 * @return True if successful.
 */
bool BLEGamepadClient::addControllerConfig(const ControllerConfig& config) {
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

ControllerInternal* BLEGamepadClient::_createController(NimBLEAddress allowedAddress) {
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

ControllerInternal* BLEGamepadClient::_getController(NimBLEAddress address) {
  for (auto& ctrl : _controllers) {
    if (ctrl.getAddress() == address) {
      return &ctrl;
    }
  }

  return nullptr;
}

bool BLEGamepadClient::_reserveController(const NimBLEAddress address) {
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

bool BLEGamepadClient::_releaseController(const NimBLEAddress address) {
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

void BLEGamepadClient::_clientStatusConsumerFn(void* pvParameters) {
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

void BLEGamepadClient::_autoScanCheck() {
  if (uxSemaphoreGetCount(_connectionSlots) == 0) {
    BLEGC_LOGD("Scan not started, no available connection slots");
    return;
  }

  BLEGC_LOGD("Scan started");
  NimBLEDevice::getScan()->start(CONFIG_BT_BLEGC_SCAN_DURATION_MS);
}

bool BLEGamepadClient::_initialized{false};
QueueHandle_t BLEGamepadClient::_clientStatusQueue{nullptr};
TaskHandle_t BLEGamepadClient::_clientStatusConsumerTask{nullptr};
SemaphoreHandle_t BLEGamepadClient::_connectionSlots{nullptr};
std::map<NimBLEAddress, uint64_t> BLEGamepadClient::_configMatch{};
std::list<ControllerInternal> BLEGamepadClient::_controllers{};
std::deque<ControllerConfig> BLEGamepadClient::_configs{};
