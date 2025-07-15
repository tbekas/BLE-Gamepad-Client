#include "BLEGamepadClient.h"
#include <NimBLEClient.h>
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEUtils.h>
#include <bitset>
#include "Logger.h"
#include "xbox.h"

#define MAX_CONFIGS 64

static constexpr uint32_t scanTimeMs = 30 * 1000;
static constexpr uint32_t connTimeoutMs = 15 * 1000;

BLEGamepadClient GamepadClient;

enum BLEClientStatusMsgKind : uint8_t {
  /// @brief BLE client bonded and connected
  BLEClientConnected = 0,

  /// @brief BLE client disconnected (bond is not deleted)
  BLEClientDisconnected = 1
};

struct BLEClientStatus {
  NimBLEAddress address;
  BLEClientStatusMsgKind kind;

  explicit operator std::string() const {
    auto kindStr = kind == BLEClientConnected ? "BLEClientConnected" : "BLEClientDisconnected";
    return "BLEClientStatus address: " + std::string(address) + ", kind: " + kindStr;
  }
};

class ClientCallbacks : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient* pClient) override {
    BLEGC_LOGD("Connected to a device, address: %s", std::string(pClient->getPeerAddress()).c_str());
    if (!pClient->secureConnection(true)) {  // async = true
      BLEGC_LOGE("Failed to initiate secure connection, address: %s", std::string(pClient->getPeerAddress()).c_str());
      // TODO: disconnect and tmp ban?
    }
  }

  void onConnectFail(NimBLEClient* pClient, int reason) override {
    BLEGC_LOGE("Failed connecting to a device, address: %s, reason: 0x%04x %s",
               std::string(pClient->getPeerAddress()).c_str(), reason, NimBLEUtils::returnCodeToString(reason));
    NimBLEDevice::deleteClient(pClient);
    if (xSemaphoreGive(GamepadClient._connectionSlots) != pdTRUE) {
      BLEGC_LOGE("Failed to release connection slot");
    }
    GamepadClient._autoScanCheck();
  }

  void onAuthenticationComplete(NimBLEConnInfo& connInfo) override {
    if (connInfo.isBonded()) {
      BLEGC_LOGI("Bonded successfully with a device, address: %s", std::string(connInfo.getAddress()).c_str());
      BLEClientStatus msg = {connInfo.getAddress(), BLEClientConnected};
      if (xQueueSend(GamepadClient._clientStatusQueue, &msg, 0) != pdPASS) {
        BLEGC_LOGE("Failed to send client status message");
      }
    } else {
      BLEGC_LOGW("Failed to bond with a device, address: %s", std::string(connInfo.getAddress()).c_str());
      // TODO: disconnect and tmp ban?
    }
    GamepadClient._autoScanCheck();
  }

  void onDisconnect(NimBLEClient* pClient, int reason) override {
    BLEGC_LOGI("Device disconnected, address: %s, reason: 0x%04x %s", std::string(pClient->getPeerAddress()).c_str(),
               reason, NimBLEUtils::returnCodeToString(reason));
    if (xSemaphoreGive(GamepadClient._connectionSlots) != pdTRUE) {
      BLEGC_LOGE("Failed to release connection slot");
    }
    GamepadClient._autoScanCheck();

    BLEClientStatus msg = {pClient->getPeerAddress(), BLEClientDisconnected};
    if (xQueueSend(GamepadClient._clientStatusQueue, &msg, 0) != pdPASS) {
      BLEGC_LOGE("Failed to send client status message");
    }
  }
} clientCallbacks;

class ScanCallbacks : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* pAdvertisedDevice) override {
    BLEGC_LOGD("Device discovered, address: %s, name: %s", std::string(pAdvertisedDevice->getAddress()).c_str(),
               pAdvertisedDevice->getName().c_str());

    auto configMatch = std::bitset<MAX_CONFIGS>();

    for (int i = 0; i < GamepadClient._configs.size(); i++) {
      auto& config = GamepadClient._configs[i];

      if (pAdvertisedDevice->haveName() && !config.deviceName.empty() &&
          pAdvertisedDevice->getName() == config.deviceName) {
        configMatch[i] = true;
        continue;
      }

      if (config.controlsConfig.isEnabled() &&
          pAdvertisedDevice->isAdvertisingService(config.controlsConfig.serviceUUID)) {
        configMatch[i] = true;
        continue;
      }

      if (config.batteryConfig.isEnabled() &&
          pAdvertisedDevice->isAdvertisingService(config.batteryConfig.serviceUUID)) {
        configMatch[i] = true;
        continue;
      }
    }

    if (configMatch.none()) {
      BLEGC_LOGD("No config found for a device");
      return;
    }

    GamepadClient._configMatch[pAdvertisedDevice->getAddress()] = configMatch.to_ulong();

    auto pClient = NimBLEDevice::getDisconnectedClient();
    if (pClient) {
      pClient->setPeerAddress(pAdvertisedDevice->getAddress());
    } else {
      pClient = NimBLEDevice::createClient(pAdvertisedDevice->getAddress());
      if (!pClient) {
        BLEGC_LOGE("Failed to create client for a device, address: %s",
                   std::string(pAdvertisedDevice->getAddress()).c_str());
        return;
      }
      pClient->setConnectTimeout(connTimeoutMs);
    }

    pClient->setClientCallbacks(&clientCallbacks, false);

    if (xSemaphoreTake(GamepadClient._connectionSlots, (TickType_t)0) != pdTRUE) {
      BLEGC_LOGD("No connections slots left");
      return;
    }

    BLEGC_LOGI("Attempting to connect to a device, address: %s", std::string(pAdvertisedDevice->getAddress()).c_str());

    if (!pClient->connect(true, true, false)) {
      BLEGC_LOGE("Failed to initiate connection, address: %s", std::string(pAdvertisedDevice->getAddress()).c_str());
      NimBLEDevice::deleteClient(pClient);
      if (xSemaphoreGive(GamepadClient._connectionSlots) != pdTRUE) {
        BLEGC_LOGE("Failed to release connection slot");
      }
      GamepadClient._autoScanCheck();
      return;
    }
  }

  void onScanEnd(const NimBLEScanResults& results, int reason) override {
    BLEGC_LOGD("Scan ended, reason: 0x%04x %s", reason, NimBLEUtils::returnCodeToString(reason));
    GamepadClient._autoScanCheck();
  }
} scanCallbacks;

Controller& BLEGamepadClient::_getOrCreateController(NimBLEAddress address) {
  for (auto& ctrl : _controllers) {
    if (ctrl.getAddress() == address) {
      BLEGC_LOGD("Reusing existing controller instance, address: %s", std::string(address).c_str());
      return ctrl;
    }
  }

  BLEGC_LOGD("Creating a new controller instance, address: %s", std::string(address).c_str());
  _controllers.emplace_back(address);
  return _controllers.back();
}

void BLEGamepadClient::_clientStatusConsumerFn(void* pvParameters) {
  auto* self = (BLEGamepadClient*)pvParameters;

  while (true) {
    BLEClientStatus msg{};
    if (xQueueReceive(self->_clientStatusQueue, &msg, portMAX_DELAY) != pdTRUE) {
      BLEGC_LOGE("Failed to receive client status message");
      return;
    }

    BLEGC_LOGD("Handling client status message %s", std::string(msg).c_str());

    switch (msg.kind) {
      case BLEClientConnected: {
        auto& ctrl = self->_getOrCreateController(msg.address);

        auto configMatch = std::bitset<MAX_CONFIGS>(self->_configMatch[msg.address]);
        for (int i = 0; i < self->_configs.size(); i++) {
          if (!configMatch[i]) {
            continue;
          }

          auto& config = self->_configs[i];

          if (!ctrl.init(config)) {
            BLEGC_LOGW("Failed to initialize controller, address: %s", std::string(msg.address).c_str());
            // this config failed, try another one
            continue;
          }

          BLEGC_LOGD("Controller sucesfuly initialized");
          self->_onConnected(ctrl);
          break;
        }
        // TODO: if all config failed, disconnect and tmp ban?
      } break;
      case BLEClientDisconnected:
        auto ctrlFound = false;
        for (auto& ctrl : self->_controllers) {
          if (ctrl.getAddress() != msg.address) {
            continue;
          }

          if (ctrl.isInitialized()) {
            if (!ctrl.deinit(true)) {
              BLEGC_LOGW("Controller failed to deinitialize, address: %s", std::string(msg.address).c_str());
            }
            BLEGC_LOGD("Controller sucesfuly deinitialized");
            self->_onDisconnected(ctrl);
          }

          ctrlFound = true;
          break;
        }

        if (!ctrlFound) {
          BLEGC_LOGW("Controller instance not found, address: %s", std::string(msg.address).c_str());
        }
        break;
    }
  }
}

void BLEGamepadClient::_autoScanCheck() {
  if (!_autoScanEnabled) {
    BLEGC_LOGD("Scan not started, auto scan is disabled");
    return;
  }

  if (uxSemaphoreGetCount(_connectionSlots) == 0) {
    BLEGC_LOGD("Scan not started, no available connection slots");
    return;
  }

  BLEGC_LOGD("Scan started");
  NimBLEDevice::getScan()->start(scanTimeMs);
}

BLEGamepadClient::BLEGamepadClient()
    :
      _initialized(false),
      _autoScanEnabled(false),
      _maxConnected(0),
      _clientStatusQueue(nullptr),
      _clientStatusConsumerTask(nullptr),
      _connectionSlots(nullptr),
      _onConnected([](Controller&) {}),
      _onDisconnected([](Controller&) {}) {
}

/**
 * @brief Initializes a GamepadClient instance.
 * @param autoScanEnabled If true, scanning starts automatically after initialization and continues until `maxConnected`
 * controllers are connected. If a controller disconnects, scanning resumes. Default is true.
 * @param maxConnected Limits the number of connected controllers. If used in conjunction with
 * `autoScanEnabled = true`, it also acts as the desired number of controllers to connect to. This means scanning will
 * continue until `maxConnected` controllers are connected. Default is 1.
 * @return True if GamepadClient instance was successfully initialized, false otherwise.
 *
 *  Example usage:
 * @code{cpp}
 * void setup(void) {
 *   // start scanning and continue until 2 controllers are connected
 *   GamepadClient.begin(true, 2);
 * }
 * @endcode
 */
bool BLEGamepadClient::begin(bool autoScanEnabled, int maxConnected) {
  if (_initialized) {
    return false;
  }

  _autoScanEnabled = autoScanEnabled;
  _maxConnected = maxConnected;
  _connectionSlots = xSemaphoreCreateCounting(_maxConnected, _maxConnected);
  configASSERT(_connectionSlots);
  _clientStatusQueue = xQueueCreate(10, sizeof(BLEClientStatus));
  configASSERT(_clientStatusQueue);

  xTaskCreatePinnedToCore(_clientStatusConsumerFn, "_bleConnectionMsgConsumerTask", 10000, this, 0,
                          &_clientStatusConsumerTask, CONFIG_BT_NIMBLE_PINNED_TO_CORE);
  configASSERT(_clientStatusConsumerTask);

  // default configs
  addConfig(xbox::controllerConfig());

  NimBLEDevice::init("Async-Client");
  NimBLEDevice::setPower(3);                                 /** +3db */
  NimBLEDevice::setSecurityAuth(true, true, true);           /** bonding, MITM protection, BLE secure connections */
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT); /** no screen, no keyboard */

  if (_autoScanEnabled) {
    NimBLEScan* pScan = NimBLEDevice::getScan();
    pScan->setScanCallbacks(&scanCallbacks, false);
    pScan->setActiveScan(true);
    pScan->setMaxResults(0);
    pScan->start(scanTimeMs);
  }

  _initialized = true;
  return true;
}

/**
 * @brief Deinitializes a GamepadClient instance.
 * @return True if successful.
 */
bool BLEGamepadClient::end() {
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

/**
 * @brief Returns all controller instances (both connected and disconnected). Use reference types when iterating with a
 * range-based for loop.
 *
 * Example usage:
 * @code{cpp}
 * void loop() {
 *   for (auto& ctrl : GamepadClient.getControllers()) {
 *     auto connStr = ctrl.isConnected() ? "connected" : "disconnected";
 *     Serial.printf("Controller is %s\n", connStr);
 *   }
 * }
 * @endcode
 */
std::list<Controller>& BLEGamepadClient::getControllers() {
  return _controllers;
}

/**
 * @brief Finds controller instance using peer address.
 * @param address Peer address of the controller to search for.
 * @return A pointer to the controller or nullptr if not found.
 */
Controller* BLEGamepadClient::getControllerPtrByAddress(NimBLEAddress address) {
  for (auto& ctrl : _controllers) {
    if (ctrl.getAddress() == address) {
      return &ctrl;
    }
  }
  return nullptr;
}

/**
 * @brief Sets the callback that will be invoked when the controller connects.
 * @param onControllerConnected Reference to a callback function.
 */
void BLEGamepadClient::setConnectedCallback(const ControllerCallback& onControllerConnected) {
  _onConnected = onControllerConnected;
}

/**
 * @brief Sets the callback that will be invoked when the controller disconnects.
 * @param onControllerDisconnected Reference to a callback function.
 */
void BLEGamepadClient::setDisconnectedCallback(const ControllerCallback& onControllerDisconnected) {
  _onDisconnected = onControllerDisconnected;
}

/**
 * @brief Registers a configuration for a new controller type. The configuration is used to set up a connection and to
 * parse raw data coming from the controller.
 * @param config Configuration to be registered.
 * @return True if successful.
 */
bool BLEGamepadClient::addConfig(const ControllerConfig& config) {
  if (_initialized) {
    BLEGC_LOGE("Failed to add config. Call `addConfig` before calling `begin`");
    return false;
  }
  if (_configs.size() >= MAX_CONFIGS) {
    BLEGC_LOGE("Failed to add config. Reached maximum number of configs: %d", _configs.size());
    return false;
  }
  if (config.controlsConfig.isDisabled() && config.batteryConfig.isDisabled()) {
    BLEGC_LOGE("Invalid config, at least one of `controlsConfig` and `batteryConfig` has to be enabled");
    return false;
  }

  _configs.push_front(config);
  return true;
}

/**
 * @brief Deletes all bonding information. Proxy for `NimBLEDevice::deleteAllBonds()`.
 * @returns True if successful.
 */
bool BLEGamepadClient::deleteAllBonds() {
  return NimBLEDevice::deleteAllBonds();
}

/**
 * @brief Starts a scan, disables auto scan. Proxy for `NimBLEDevice::getScan()->start()`.
 * @param durationMs The duration in milliseconds for which to scan. 0 == scan forever.
 * @return True if successful.
 */
bool BLEGamepadClient::startScan(uint32_t durationMs) {
  _autoScanEnabled = false;
  return NimBLEDevice::getScan()->start(durationMs);
}

/**
 * @brief Stops a scan, disables auto scan. Proxy for `NimBLEDevice::getScan()->stop()`.
 * @return True if successful.
 */
bool BLEGamepadClient::stopScan() {
  _autoScanEnabled = false;
  return NimBLEDevice::getScan()->stop();
}
