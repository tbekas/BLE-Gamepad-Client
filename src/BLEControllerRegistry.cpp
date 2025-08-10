#include "BLEControllerRegistry.h"

#include <NimBLEClient.h>
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEUtils.h>
#include <bitset>
#include "config.h"
#include "logger.h"

static auto* LOG_TAG = "BLEControllerRegistry";

BLEClientStatus::operator std::string() const {
  auto kindStr = kind == BLEClientConnected ? "BLEClientConnected" : "BLEClientDisconnected";
  return "BLEClientStatus address: " + std::string(address) + ", kind: " + kindStr;
}

BLEControllerRegistry::BLEControllerRegistry(TaskHandle_t& autoScanTask, BLEControllerMatcher& matcher)
    : _initialized(false),
      _autoScanTask(autoScanTask),
      _matcher(matcher),
      _clientStatusQueue(nullptr),
      _clientStatusConsumerTask(nullptr),
      _connectionSlots(nullptr),
      _clientCallbacks(*this) {}

/**
 * @brief Initializes a BLEControllerRegistry.
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

  xTaskCreatePinnedToCore(_clientStatusConsumerFn, "_bleConnectionMsgConsumerTask", 10000, this, 0,
                          &_clientStatusConsumerTask, CONFIG_BT_NIMBLE_PINNED_TO_CORE);
  configASSERT(_clientStatusConsumerTask);

  _initialized = true;
  return true;
}

/**
 * @brief Deinitializes a BLEControllerRegistry.
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

    auto* pClient = NimBLEDevice::getClientByPeerAddress(ctrl.getAddress());
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
 * @brief Checks if BLEControllerRegistry is initialized.
 * @return True if initialized; false otherwise.
 */
bool BLEControllerRegistry::isInitialized() {
  return _initialized;
}

BLEControllerInternal* BLEControllerRegistry::createController(NimBLEAddress allowedAddress) {
  if (xSemaphoreGive(_connectionSlots) != pdTRUE) {
    BLEGC_LOGE(LOG_TAG, "Cannot create controller");
    return nullptr;
  }

  BLEGC_LOGD(LOG_TAG, "Creating a new controller instance, address: %s", std::string(allowedAddress).c_str());
  _controllers.emplace_back(allowedAddress);
  auto& ctrl = _controllers.back();

  xTaskNotifyGive(_autoScanTask);
  return &ctrl;
}
void BLEControllerRegistry::connectController(NimBLEAddress address) {
  if (!_reserveController(address)) {
    return;
  }

  auto* pClient = NimBLEDevice::getClientByPeerAddress(address);
  if (pClient) {
    BLEGC_LOGD(LOG_TAG, "Reusing existing client for a device, address: %s",
               std::string(pClient->getPeerAddress()).c_str());
  } else {
    pClient = NimBLEDevice::createClient(address);
    if (!pClient) {
      BLEGC_LOGE(LOG_TAG, "Failed to create client for a device, address: %s", std::string(address).c_str());
      _releaseController(address);
      xTaskNotifyGive(_autoScanTask);
      return;
    }
    pClient->setConnectTimeout(CONFIG_BT_BLEGC_CONN_TIMEOUT_MS);
    pClient->setClientCallbacks(&_clientCallbacks, false);
  }

  BLEGC_LOGI(LOG_TAG, "Attempting to connect to a device, address: %s", std::string(pClient->getPeerAddress()).c_str());

  if (!pClient->connect(true, true, true)) {
    BLEGC_LOGE(LOG_TAG, "Failed to initiate connection, address: %s", std::string(pClient->getPeerAddress()).c_str());
    NimBLEDevice::deleteClient(pClient);
    _releaseController(pClient->getPeerAddress());
    xTaskNotifyGive(_autoScanTask);
    return;
  }
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
    BLEGC_LOGD(LOG_TAG, "No connections slots left");
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

  BLEGC_LOGD(LOG_TAG, "No suitable controller available");

  if (xSemaphoreGive(_connectionSlots) != pdTRUE) {
    BLEGC_LOGE(LOG_TAG, "Failed to release connection slot");
    return false;
  }

  return false;
}

bool BLEControllerRegistry::_releaseController(const NimBLEAddress address) {
  auto* pCtrl = _getController(address);
  if (pCtrl == nullptr) {
    BLEGC_LOGE(LOG_TAG, "Controller not found, address: %s", std::string(address).c_str());
    return false;
  }

  pCtrl->setLastAddress(pCtrl->getAddress());
  pCtrl->setAddress(NimBLEAddress());

  if (xSemaphoreGive(_connectionSlots) != pdTRUE) {
    BLEGC_LOGE(LOG_TAG, "Failed to release connection slot");
    return false;
  }
  return true;
}

unsigned int BLEControllerRegistry::getAvailableConnectionSlotCount() const {
  if (!_initialized) {
    return 0;
  }

  return uxSemaphoreGetCount(_connectionSlots);
}

void BLEControllerRegistry::_clientStatusConsumerFn(void* pvParameters) {
  auto* self = static_cast<BLEControllerRegistry*>(pvParameters);

  while (true) {
    BLEClientStatus msg{};
    if (xQueueReceive(self->_clientStatusQueue, &msg, portMAX_DELAY) != pdTRUE) {
      BLEGC_LOGE(LOG_TAG, "Failed to receive client status message");
      return;
    }

    BLEGC_LOGD(LOG_TAG, "Handling client status message %s", std::string(msg).c_str());

    switch (msg.kind) {
      case BLEClientConnected: {
        auto* pCtrl = self->_getController(msg.address);
        if (pCtrl == nullptr) {
          BLEGC_LOGE(LOG_TAG, "Controller not found, address: %s", std::string(msg.address).c_str());
          break;
        }

        auto modelMatch = std::bitset<MAX_CTRL_MODEL_COUNT>(self->_matcher.getMatchedModels(msg.address));
        const unsigned int modelCount = self->_matcher.getModelCount();
        configASSERT(modelCount > 0);

        // iterate over models from back to front
        unsigned int i = modelCount;
        do {
          i--;

          if (!modelMatch[i]) {
            continue;
          }

          auto& model = self->_matcher.getModel(i);

          if (!pCtrl->init(model)) {
            BLEGC_LOGW(LOG_TAG, "Failed to initialize controller, address: %s", std::string(msg.address).c_str());
            // this config failed, try another one
            continue;
          }

          BLEGC_LOGD(LOG_TAG, "Controller successfully initialized");
          break;

        } while (i > 0);

        // TODO: disconnect and tmp ban?
      } break;
      case BLEClientDisconnected:
        auto* pCtrl = self->_getController(msg.address);
        if (pCtrl == nullptr) {
          BLEGC_LOGE(LOG_TAG, "Controller not found, address: %s", std::string(msg.address).c_str());
          break;
        }

        if (pCtrl->isInitialized()) {
          if (!pCtrl->deinit(true)) {
            BLEGC_LOGW(LOG_TAG, "Controller failed to deinitialize, address: %s", std::string(msg.address).c_str());
          }
          BLEGC_LOGD(LOG_TAG, "Controller successfully deinitialized");
        }

        self->_releaseController(msg.address);
        xTaskNotifyGive(self->_autoScanTask);
        break;
    }
  }
}

BLEControllerRegistry::ClientCallbacks::ClientCallbacks(BLEControllerRegistry& controllerRegistry)
    : _controllerRegistry(controllerRegistry) {}

void BLEControllerRegistry::ClientCallbacks::onConnect(NimBLEClient* pClient) {
  BLEGC_LOGD(LOG_TAG, "Connected to a device, address: %s", std::string(pClient->getPeerAddress()).c_str());
  if (!pClient->secureConnection(true)) {  // async = true
    BLEGC_LOGE(LOG_TAG, "Failed to initiate secure connection, address: %s",
               std::string(pClient->getPeerAddress()).c_str());
    // TODO: disconnect and tmp ban?
  }
}

void BLEControllerRegistry::ClientCallbacks::onConnectFail(NimBLEClient* pClient, int reason) {
  BLEGC_LOGE(LOG_TAG, "Failed connecting to a device, address: %s, reason: 0x%04x %s",
             std::string(pClient->getPeerAddress()).c_str(), reason, NimBLEUtils::returnCodeToString(reason));
  NimBLEDevice::deleteClient(pClient);
  _controllerRegistry._releaseController(pClient->getPeerAddress());
  xTaskNotifyGive(_controllerRegistry._autoScanTask);
}

void BLEControllerRegistry::ClientCallbacks::onAuthenticationComplete(NimBLEConnInfo& connInfo) {
  if (connInfo.isBonded()) {
    BLEGC_LOGI(LOG_TAG, "Bonded successfully with a device, address: %s", std::string(connInfo.getAddress()).c_str());
    BLEClientStatus msg = {connInfo.getAddress(), BLEClientConnected};
    if (xQueueSend(_controllerRegistry._clientStatusQueue, &msg, 0) != pdPASS) {
      BLEGC_LOGE(LOG_TAG, "Failed to send client status message");
    }
  } else {
    BLEGC_LOGW(LOG_TAG, "Failed to bond with a device, address: %s", std::string(connInfo.getAddress()).c_str());
    // TODO: disconnect and tmp ban?
  }
  xTaskNotifyGive(_controllerRegistry._autoScanTask);
}

void BLEControllerRegistry::ClientCallbacks::onDisconnect(NimBLEClient* pClient, int reason) {
  BLEGC_LOGI(LOG_TAG, "Device disconnected, address: %s, reason: 0x%04x %s",
             std::string(pClient->getPeerAddress()).c_str(), reason, NimBLEUtils::returnCodeToString(reason));
  BLEClientStatus msg = {pClient->getPeerAddress(), BLEClientDisconnected};
  if (xQueueSend(_controllerRegistry._clientStatusQueue, &msg, 0) != pdPASS) {
    BLEGC_LOGE(LOG_TAG, "Failed to send client status message");
  }
}
