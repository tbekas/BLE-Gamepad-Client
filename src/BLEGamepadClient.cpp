#include "BLEGamepadClient.h"
#include <NimBLEClient.h>
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEUtils.h>
#include <bitset>
#include "config.h"
#include "logger.h"
#include "xbox.h"

#define MAX_CTRL_CONFIG_COUNT sizeof(CTRL_CONFIG_MATCH_TYPE)

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
    BLEGamepadClient::_releaseController(pClient->getPeerAddress());
    BLEGamepadClient::_autoScanCheck();
  }

  void onAuthenticationComplete(NimBLEConnInfo& connInfo) override {
    if (connInfo.isBonded()) {
      BLEGC_LOGI("Bonded successfully with a device, address: %s", std::string(connInfo.getAddress()).c_str());
      BLEClientStatus msg = {connInfo.getAddress(), BLEClientConnected};
      if (xQueueSend(BLEGamepadClient::_clientStatusQueue, &msg, 0) != pdPASS) {
        BLEGC_LOGE("Failed to send client status message");
      }
    } else {
      BLEGC_LOGW("Failed to bond with a device, address: %s", std::string(connInfo.getAddress()).c_str());
      // TODO: disconnect and tmp ban?
    }
    BLEGamepadClient::_autoScanCheck();
  }

  void onDisconnect(NimBLEClient* pClient, int reason) override {
    BLEGC_LOGI("Device disconnected, address: %s, reason: 0x%04x %s", std::string(pClient->getPeerAddress()).c_str(),
               reason, NimBLEUtils::returnCodeToString(reason));
    BLEClientStatus msg = {pClient->getPeerAddress(), BLEClientDisconnected};
    if (xQueueSend(BLEGamepadClient::_clientStatusQueue, &msg, 0) != pdPASS) {
      BLEGC_LOGE("Failed to send client status message");
    }
  }
} clientCallbacks;

class ScanCallbacks : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* pAdvertisedDevice) override {
    BLEGC_LOGD("Device discovered, address: %s, name: %s", std::string(pAdvertisedDevice->getAddress()).c_str(),
               pAdvertisedDevice->getName().c_str());

    auto configMatch = std::bitset<MAX_CTRL_CONFIG_COUNT>();

    for (int i = 0; i < BLEGamepadClient::_configs.size(); i++) {
      auto& config = BLEGamepadClient::_configs[i];

      if (pAdvertisedDevice->haveName() && !config.deviceName.empty() &&
          pAdvertisedDevice->getName() == config.deviceName) {
        configMatch[i] = true;
        continue;
      }

      if (config.controls.isEnabled() && pAdvertisedDevice->isAdvertisingService(config.controls.serviceUUID)) {
        configMatch[i] = true;
        continue;
      }

      if (config.battery.isEnabled() && pAdvertisedDevice->isAdvertisingService(config.battery.serviceUUID)) {
        configMatch[i] = true;
        continue;
      }

      if (config.vibrations.isEnabled() && pAdvertisedDevice->isAdvertisingService(config.vibrations.serviceUUID)) {
        configMatch[i] = true;
        continue;
      }
    }

    if (configMatch.none()) {
      BLEGC_LOGD("No config found for a device");
      return;
    }

    if (!BLEGamepadClient::_reserveController(pAdvertisedDevice->getAddress())) {
      return;
    }

    BLEGamepadClient::_configMatch[pAdvertisedDevice->getAddress()] = configMatch.to_ulong();

    auto pClient = NimBLEDevice::getClientByPeerAddress(pAdvertisedDevice->getAddress());
    if (pClient) {
      BLEGC_LOGD("Reusing existing client for a device, address: %s", std::string(pClient->getPeerAddress()).c_str());
    } else {
      pClient = NimBLEDevice::createClient(pAdvertisedDevice->getAddress());
      if (!pClient) {
        BLEGC_LOGE("Failed to create client for a device, address: %s",
                   std::string(pAdvertisedDevice->getAddress()).c_str());
        BLEGamepadClient::_releaseController(pAdvertisedDevice->getAddress());
        BLEGamepadClient::_autoScanCheck();
        return;
      }
      pClient->setConnectTimeout(CONFIG_BT_BLEGC_CONN_TIMEOUT_MS);
      pClient->setClientCallbacks(&clientCallbacks, false);
    }

    BLEGC_LOGI("Attempting to connect to a device, address: %s", std::string(pClient->getPeerAddress()).c_str());

    if (!pClient->connect(true, true, false)) {
      BLEGC_LOGE("Failed to initiate connection, address: %s", std::string(pClient->getPeerAddress()).c_str());
      NimBLEDevice::deleteClient(pClient);
      BLEGamepadClient::_releaseController(pClient->getPeerAddress());
      BLEGamepadClient::_autoScanCheck();
      return;
    }
  }

  void onScanEnd(const NimBLEScanResults& results, int reason) override {
    BLEGC_LOGD("Scan ended, reason: 0x%04x %s", reason, NimBLEUtils::returnCodeToString(reason));
    BLEGamepadClient::_autoScanCheck();
  }
} scanCallbacks;

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

bool BLEGamepadClient::_reserveController(const NimBLEAddress address) {
  if (xSemaphoreTake(_connectionSlots, 0) != pdTRUE) {
    BLEGC_LOGD("No connections slots left");
    return false;
  }

  // get one with matching filter
  for (auto& ctrl : _controllers) {
    if (!ctrl.getAddress().isNull()) {
      // already reserved
      continue;
    }

    if (!ctrl.getAllowedAddress().isNull() && ctrl.getAllowedAddress() == address) {
      ctrl.setAddress(address);
      return true;
    }
  }

  // get one reserved last time
  for (auto& ctrl : _controllers) {
    if (!ctrl.getAddress().isNull()) {
      // already reserved
      continue;
    }

    if (!ctrl.getLastAddress().isNull() && ctrl.getLastAddress() == address) {
      ctrl.setAddress(address);
      return true;
    }
  }

  // get any
  for (auto& ctrl : _controllers) {
    if (!ctrl.getAddress().isNull()) {
      // already reserved
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
        // reverse iteration order - configs
        for (int i = _configs.size() - 1; i >= 0; i--) {
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
        // TODO: if all config failed, disconnect and tmp ban?
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

bool BLEGamepadClient::isInitialized() {
  return _initialized;
}

bool BLEGamepadClient::init(bool deleteBonds) {
  if (_initialized) {
    return false;
  }

  _connectionSlots = xSemaphoreCreateCounting(CONFIG_BT_BLEGC_MAX_CONNECTIONS, 0);
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

  if (deleteBonds) {
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
