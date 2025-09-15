#include "BLEControllerRegistry.h"

#include <NimBLEClient.h>
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEUtils.h>
#include <algorithm>
#include <bitset>
#include <memory>
#include "BLEBaseController.h"
#include "config.h"
#include "logger.h"

static auto* LOG_TAG = "BLEControllerRegistry";

BLEClientStatus::operator std::string() const {
  auto kindStr = kind == BLEClientConnected ? "BLEClientConnected" : "BLEClientDisconnected";
  return "BLEClientStatus address: " + std::string(address) + ", kind: " + kindStr;
}

BLEControllerRegistry::BLEControllerRegistry(TaskHandle_t& autoScanTask)
    : _autoScanTask(autoScanTask),
      _callbackTask(nullptr),
      _clientStatusQueue(nullptr),
      _clientStatusConsumerTask(nullptr),
      _connectionSlots(nullptr),
      _clientCallbacks(*this),
      _controllers(new std::vector<BLEBaseController*>()){
  xTaskCreate(_callbackTaskFn, "_callbackTask", 10000, this, 0, &_callbackTask);
  configASSERT(_callbackTask);

  _connectionSlots = xSemaphoreCreateCounting(CONFIG_BT_BLEGC_MAX_CONNECTION_SLOTS, 0);
  configASSERT(_connectionSlots);
  _clientStatusQueue = xQueueCreate(10, sizeof(BLEClientStatus));
  configASSERT(_clientStatusQueue);

  xTaskCreatePinnedToCore(_clientStatusConsumerFn, "_bleConnectionMsgConsumerTask", 10000, this, 0,
                          &_clientStatusConsumerTask, CONFIG_BT_NIMBLE_PINNED_TO_CORE);
  configASSERT(_clientStatusConsumerTask);
}

BLEControllerRegistry::~BLEControllerRegistry() {
  if (_clientStatusConsumerTask != nullptr) {
    vTaskDelete(_clientStatusConsumerTask);
  }
  if (_clientStatusQueue != nullptr) {
    vQueueDelete(_clientStatusQueue);
  }
  if (_connectionSlots != nullptr) {
    vSemaphoreDelete(_connectionSlots);
  }

  const auto* pControllers = _controllers.load();
  delete pControllers;
}

void BLEControllerRegistry::registerController(BLEBaseController* pCtrl) {
  auto* pControllersOld = _controllers.load();
  auto* pControllersNew = new std::vector<BLEBaseController*>();
  do {
    pControllersNew->clear();
    for (auto* pSomeCtrl : *pControllersOld) {
      pControllersNew->push_back(pSomeCtrl);
      if (pSomeCtrl == pCtrl) {
        BLEGC_LOGD(LOG_TAG, "Controller already registered");
        return;
      }
    }
    pControllersNew->push_back(pCtrl);
  } while (!_controllers.compare_exchange_weak(pControllersOld,
                                               pControllersNew));  // this loads updated data into controllersOld on failure

  delete pControllersOld;

  if (pControllersNew->size() <= CONFIG_BT_BLEGC_MAX_CONNECTION_SLOTS) {
    if (xSemaphoreGive(_connectionSlots) == pdTRUE) {
      xTaskNotifyGive(_autoScanTask);
    } else {
      BLEGC_LOGE(LOG_TAG, "Cannot add a connection slot");
    }
  } else {
    BLEGC_LOGD(LOG_TAG,
               "Max amount of connection slots reached. Increase CONFIG_BT_BLEGC_MAX_CONNECTION_SLOTS to add more.");
  }
}

void BLEControllerRegistry::deregisterController(BLEBaseController* pCtrl) {
  pCtrl->markPendingDeregistration();

  if (pCtrl->isConnected()) {
    pCtrl->disconnect();
    return; // deregistration will continue after completing disconnect
  }

  if (pCtrl->isReserved()) {
    // initialization is in progress
    return; // deregistration will continue after completing initialization and disconnect
  }

  // continuing deregistration
  auto* pControllersOld = _controllers.load();
  auto* pControllersNew = new std::vector<BLEBaseController*>();
  do {
    pControllersNew->clear();
    bool found = false;
    for (auto* pSomeCtrl : *pControllersOld) {
      if (pSomeCtrl == pCtrl) {
        found = true;
        continue;
      }
      pControllersNew->push_back(pSomeCtrl);
    }

    if (!found) {
      BLEGC_LOGD(LOG_TAG, "Controller not registered");
      return;
    }
  } while (!_controllers.compare_exchange_weak(pControllersOld,
                                               pControllersNew));  // this loads updated data into controllersOld

  delete pControllersOld;

  if (pControllersNew->size() < CONFIG_BT_BLEGC_MAX_CONNECTION_SLOTS) {
    if (xSemaphoreTake(_connectionSlots, 0) != pdTRUE) {
      BLEGC_LOGE(LOG_TAG, "Cannot remove a connection slot");
    }
  }

  pCtrl->markCompletedDeregistration();
}

void BLEControllerRegistry::tryConnectController(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  if (!_reserveController(pAdvertisedDevice)) {
    return;
  }

  const auto address = pAdvertisedDevice->getAddress();

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

BLEBaseController* BLEControllerRegistry::_getController(const NimBLEAddress address) const {
  for (auto* pCtrl : *_controllers.load()) {
    if (pCtrl->getAddress() == address) {
      return pCtrl;
    }
  }

  return nullptr;
}

bool BLEControllerRegistry::_reserveController(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  if (xSemaphoreTake(_connectionSlots, 0) != pdTRUE) {
    BLEGC_LOGD(LOG_TAG, "No connections slots left");
    return false;
  }

  const auto address = pAdvertisedDevice->getAddress();

  // try reserve ctrl with matching allowed address
  for (auto* pCtrl : *_controllers.load()) {
    if (pCtrl->isReserved() || !pCtrl->isSupported(pAdvertisedDevice) || pCtrl->isPendingDeregistration()) {
      continue;
    }

    if (!pCtrl->getAllowedAddress().isNull() && pCtrl->getAllowedAddress() == address) {
      pCtrl->setAddress(address);
      return true;
    }
  }

  // try reserve ctrl reserved last time
  for (auto* pCtrl : *_controllers.load()) {
    if (pCtrl->isReserved() || !pCtrl->isSupported(pAdvertisedDevice) || pCtrl->isPendingDeregistration()) {
      continue;
    }

    if (!pCtrl->getLastAddress().isNull() && pCtrl->getLastAddress() == address) {
      pCtrl->setAddress(address);
      return true;
    }
  }

  // try reserve any controller
  for (auto* pCtrl : *_controllers.load()) {
    if (pCtrl->isReserved() || !pCtrl->isSupported(pAdvertisedDevice) || pCtrl->isPendingDeregistration()) {
      continue;
    }

    if (pCtrl->getAllowedAddress().isNull()) {
      pCtrl->setAddress(address);
      return true;
    }
  }

  BLEGC_LOGD(LOG_TAG, "No suitable controller available for a device, address %s", std::string(address).c_str());

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

  BLEGC_LOGD(LOG_TAG, "Releasing controller %s", std::string(address).c_str());

  pCtrl->setLastAddress(pCtrl->getAddress());
  pCtrl->setAddress(NimBLEAddress());

  if (xSemaphoreGive(_connectionSlots) != pdTRUE) {
    BLEGC_LOGE(LOG_TAG, "Failed to release connection slot");
    return false;
  }
  return true;
}

unsigned int BLEControllerRegistry::getAvailableConnectionSlotCount() const {
  return uxSemaphoreGetCount(_connectionSlots);
}

void BLEControllerRegistry::_callbackTaskFn(void* pvParameters) {
  while (true) {
    const auto val = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    auto* pCtrl = reinterpret_cast<BLEBaseController*>(val);
    if (pCtrl->isConnected()) {
      pCtrl->callOnConnect();
    } else {
      pCtrl->callOnDisconnect();
    }
  }
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

    auto* pCtrl = self->_getController(msg.address);
    if (!pCtrl) {
      BLEGC_LOGE(LOG_TAG, "Controller not found, address: %s", std::string(msg.address).c_str());
      break;
    }

    switch (msg.kind) {
      case BLEClientConnected: {
        auto* pClient = NimBLEDevice::getClientByPeerAddress(msg.address);
        if (!pClient) {
          BLEGC_LOGE(LOG_TAG, "BLE client not found, address %s", std::string(msg.address).c_str());
          break;
        }

        if (!pCtrl->init(pClient)) {
          BLEGC_LOGW(LOG_TAG, "Failed to initialize controller, address: %s", std::string(msg.address).c_str());
          // TODO: disconnect and tmp ban?
          break;
        }

        BLEGC_LOGD(LOG_TAG, "Controller successfully initialized");

        pCtrl->markConnected();
        xTaskNotify(self->_callbackTask, reinterpret_cast<uint32_t>(pCtrl), eSetValueWithOverwrite);

        if (pCtrl->isPendingDeregistration()) {
          pCtrl->disconnect();
        }

        break;
      }
      case BLEClientDisconnected:
        if (!pCtrl->deinit()) {
          BLEGC_LOGW(LOG_TAG, "Controller failed to deinitialize, address: %s", std::string(msg.address).c_str());
        }

        self->_releaseController(msg.address);
        pCtrl->markDisconnected();

        if (pCtrl->isPendingDeregistration()) {
          self->deregisterController(pCtrl);
        }

        xTaskNotifyGive(self->_autoScanTask);
        xTaskNotify(self->_callbackTask, reinterpret_cast<uint32_t>(pCtrl), eSetValueWithOverwrite);

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
