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

BLEClientEvent::operator std::string() const {
  std::string kindStr;
  // clang-format off
  switch (kind) {
    case BLEClientBonded: kindStr = "BLEClientBonded"; break;
    case BLEClientDisconnected: kindStr = "BLEClientDisconnected"; break;
    case BLEClientConnectingFailed: kindStr = "BLEClientConnectingFailed"; break;
    case BLEClientBondingFailed: kindStr = "BLEClientBondingFailed"; break;
  }
  // clang-format on
  return "BLEClientStatus address: " + std::string(address) + ", kind: " + kindStr;
}

BLEControllerRegistry::BLEControllerRegistry(TaskHandle_t& autoScanTask)
    : _autoScanTask(autoScanTask),
      _callbackTask(nullptr),
      _clientEventQueue(nullptr),
      _clientEventConsumerTask(nullptr),
      _connectionSlots(nullptr),
      _clientCallbacks(*this),
      _controllers(new std::vector<BLEAbstractController*>()) {
  xTaskCreate(_callbackTaskFn, "_callbackTask", 10000, this, 0, &_callbackTask);
  configASSERT(_callbackTask);

  _connectionSlots = xSemaphoreCreateCounting(CONFIG_BT_BLEGC_MAX_CONNECTION_SLOTS, 0);
  configASSERT(_connectionSlots);
  _clientEventQueue = xQueueCreate(10, sizeof(BLEClientEvent));
  configASSERT(_clientEventQueue);

  xTaskCreatePinnedToCore(_clientEventConsumerFn, "_clientEventConsumerTask", 10000, this, 0,
                          &_clientEventConsumerTask, CONFIG_BT_NIMBLE_PINNED_TO_CORE);
  configASSERT(_clientEventConsumerTask);
}

BLEControllerRegistry::~BLEControllerRegistry() {
  if (_clientEventConsumerTask != nullptr) {
    vTaskDelete(_clientEventConsumerTask);
  }
  if (_clientEventQueue != nullptr) {
    vQueueDelete(_clientEventQueue);
  }
  if (_connectionSlots != nullptr) {
    vSemaphoreDelete(_connectionSlots);
  }

  const auto* pControllers = _controllers.load();
  delete pControllers;
}

void BLEControllerRegistry::registerController(BLEAbstractController* pCtrl) {
  auto* pControllersOld = _controllers.load();
  auto* pControllersNew = new std::vector<BLEAbstractController*>();
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
  } while (
      !_controllers.compare_exchange_weak(pControllersOld,
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
  BLEGC_LOGD(LOG_TAG, "Controller registered");
}

void BLEControllerRegistry::deregisterController(BLEAbstractController* pCtrl) {
  pCtrl->markPendingDeregistration();

  auto* pClient = pCtrl->getClient();
  if (pClient && pClient->isConnected()) {
    pClient->disconnect();
    return;  // deregistration will continue after completing disconnect
  }

  if (pCtrl->isAllocated() && (!pClient || !pClient->isConnected())) {
    // connect is likely in progress, we can try to cancel it
    auto* pClient = pCtrl->getClient();
    if (pClient) {
      if (pClient->cancelConnect()) {
        BLEGC_LOGD(LOG_TAG, "Cancel connect command sent successfully");
      } else {
        BLEGC_LOGD(LOG_TAG, "Cancel connect command failed");
      }
    }
    return;  // deregistration will continue after canceling connection
  }

  auto* pControllersOld = _controllers.load();
  auto* pControllersNew = new std::vector<BLEAbstractController*>();
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
    if (xSemaphoreTake(_connectionSlots, 0) == pdTRUE) {
      xTaskNotifyGive(_autoScanTask);
    } else {
      BLEGC_LOGE(LOG_TAG, "Cannot remove a connection slot");
    }
  }

  pCtrl->markCompletedDeregistration();
  BLEGC_LOGD(LOG_TAG, "Controller deregistered");
}

void BLEControllerRegistry::tryConnectController(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  if (!_allocateController(pAdvertisedDevice)) {
    return;
  }

  const auto address = pAdvertisedDevice->getAddress();

  auto* pClient = NimBLEDevice::getClientByPeerAddress(address);
  if (pClient) {
    BLEGC_LOGD(LOG_TAG, "Using existing client for a device, address: %s", std::string(address).c_str());
  } else {
    pClient = NimBLEDevice::getDisconnectedClient();

    if (pClient) {
      BLEGC_LOGD(LOG_TAG, "Reusing disconnected client for a device, address: %s", std::string(address).c_str());
      pClient->setPeerAddress(address);
    } else{
      BLEGC_LOGD(LOG_TAG, "Creating a client for a device, address: %s", std::string(address).c_str());
      pClient = NimBLEDevice::createClient(address);

      if (!pClient) {
        BLEGC_LOGE(LOG_TAG, "Failed to create client for a device, address: %s", std::string(address).c_str());

        _sendClientEvent({address, BLEClientConnectingFailed});
        return;
      }

      pClient->setConnectTimeout(CONFIG_BT_BLEGC_CONN_TIMEOUT_MS);
      pClient->setClientCallbacks(&_clientCallbacks, false);
    }
  }

  BLEGC_LOGI(LOG_TAG, "Attempting to connect to a device, address: %s", std::string(pClient->getPeerAddress()).c_str());

  if (!pClient->connect(true, true, true)) {
    BLEGC_LOGE(LOG_TAG, "Failed to initiate connection, address: %s", std::string(pClient->getPeerAddress()).c_str());
    _sendClientEvent({address, BLEClientConnectingFailed});
  }
}

BLEAbstractController* BLEControllerRegistry::_getController(const NimBLEAddress address) const {
  for (auto* pCtrl : *_controllers.load()) {
    if (pCtrl->getAddress() == address) {
      return pCtrl;
    }
  }

  BLEGC_LOGE(LOG_TAG, "Controller not found, address: %s", std::string(address).c_str());
  return nullptr;
}

bool BLEControllerRegistry::_allocateController(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  if (xSemaphoreTake(_connectionSlots, 0) != pdTRUE) {
    BLEGC_LOGD(LOG_TAG, "No connections slots left");
    return false;
  }

  const auto address = pAdvertisedDevice->getAddress();

  // allocate ctrl with matching allowed address
  for (auto* pCtrl : *_controllers.load()) {
    if (pCtrl->isAllocated() || !pCtrl->isSupported(pAdvertisedDevice) || pCtrl->isPendingDeregistration()) {
      continue;
    }

    if (!pCtrl->getAllowedAddress().isNull() && pCtrl->getAllowedAddress() == address) {
      pCtrl->setAddress(address);
      return true;
    }
  }

  // allocate ctrl allocated last time
  for (auto* pCtrl : *_controllers.load()) {
    if (pCtrl->isAllocated() || !pCtrl->isSupported(pAdvertisedDevice) || pCtrl->isPendingDeregistration()) {
      continue;
    }

    if (!pCtrl->getLastAddress().isNull() && pCtrl->getLastAddress() == address) {
      pCtrl->setAddress(address);
      return true;
    }
  }

  // allocate any controller
  for (auto* pCtrl : *_controllers.load()) {
    if (pCtrl->isAllocated() || !pCtrl->isSupported(pAdvertisedDevice) || pCtrl->isPendingDeregistration()) {
      continue;
    }

    if (pCtrl->getAllowedAddress().isNull()) {
      pCtrl->setAddress(address);
      return true;
    }
  }

  BLEGC_LOGD(LOG_TAG, "No suitable controller to allocate for a device, address %s", std::string(address).c_str());

  if (xSemaphoreGive(_connectionSlots) != pdTRUE) {
    BLEGC_LOGE(LOG_TAG, "Failed to release connection slot");
    return false;
  }

  return false;
}

bool BLEControllerRegistry::_deallocateController(BLEAbstractController* pCtrl) {
  if (!pCtrl->isAllocated()) {
    BLEGC_LOGE(LOG_TAG, "Attempting to deallocate controller that is not allocated, last address: %s",
               std::string(pCtrl->getLastAddress()).c_str());
    return false;
  }

  BLEGC_LOGD(LOG_TAG, "Deallocating controller %s", std::string(pCtrl->getAddress()).c_str());

  pCtrl->setLastAddress(pCtrl->getAddress());
  pCtrl->setAddress(NimBLEAddress());

  if (xSemaphoreGive(_connectionSlots) != pdTRUE) {
    BLEGC_LOGE(LOG_TAG, "Failed to release connection slot");
    return false;
  }
  return true;
}
void BLEControllerRegistry::_sendClientEvent(const BLEClientEvent& msg) const {
  if (xQueueSend(_clientEventQueue, &msg, 0) != pdPASS) {
    BLEGC_LOGE(LOG_TAG, "Failed to send client status message");
  }
}

unsigned int BLEControllerRegistry::getAvailableConnectionSlotCount() const {
  return uxSemaphoreGetCount(_connectionSlots);
}

void BLEControllerRegistry::_callbackTaskFn(void* pvParameters) {
  while (true) {
    const auto val = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    auto* pCtrl = reinterpret_cast<BLEAbstractController*>(val);
    if (pCtrl->isConnected()) {
      pCtrl->callOnConnect();
    } else {
      pCtrl->callOnDisconnect();
    }
  }
}

void BLEControllerRegistry::_clientEventConsumerFn(void* pvParameters) {
  auto* self = static_cast<BLEControllerRegistry*>(pvParameters);

  while (true) {
    BLEClientEvent msg{};
    if (xQueueReceive(self->_clientEventQueue, &msg, portMAX_DELAY) != pdTRUE) {
      BLEGC_LOGE(LOG_TAG, "Failed to receive client status message");
      return;
    }

    BLEGC_LOGD(LOG_TAG, "Handling client status message %s", std::string(msg).c_str());

    auto* pCtrl = self->_getController(msg.address);
    if (!pCtrl) {
      break;
    }

    switch (msg.kind) {
      case BLEClientBonded: {
        if (pCtrl->isPendingDeregistration()) {
          pCtrl->getClient()->disconnect();
          break;
        }

        if (!pCtrl->init(pCtrl->getClient())) {
          BLEGC_LOGW(LOG_TAG, "Failed to initialize controller, address: %s", std::string(msg.address).c_str());
          pCtrl->getClient()->disconnect();
          break;
        }

        pCtrl->markConnected();
        xTaskNotify(self->_callbackTask, reinterpret_cast<uint32_t>(pCtrl), eSetValueWithOverwrite);

        BLEGC_LOGD(LOG_TAG, "Controller successfully initialized");

        break;
      }
      case BLEClientDisconnected: {
        if (!pCtrl->deinit()) {
          BLEGC_LOGW(LOG_TAG, "Failed to deinitialize controller, address: %s", std::string(msg.address).c_str());
        }

        self->_deallocateController(pCtrl);

        if (pCtrl->isConnected()) {
          pCtrl->markDisconnected();
          xTaskNotify(self->_callbackTask, reinterpret_cast<uint32_t>(pCtrl), eSetValueWithOverwrite);
        }

        if (pCtrl->isPendingDeregistration()) {
          self->deregisterController(pCtrl);
        }
        break;
      }
      case BLEClientConnectingFailed: {
        self->_deallocateController(pCtrl);

        if (pCtrl->isPendingDeregistration()) {
          self->deregisterController(pCtrl);
        }
        break;
      }
      case BLEClientBondingFailed: {
        pCtrl->getClient()->disconnect();
        break;
      }
    }
    xTaskNotifyGive(self->_autoScanTask);
  }
}

BLEControllerRegistry::ClientCallbacks::ClientCallbacks(BLEControllerRegistry& controllerRegistry)
    : _controllerRegistry(controllerRegistry) {}

void BLEControllerRegistry::ClientCallbacks::onConnect(NimBLEClient* pClient) {
  BLEGC_LOGD(LOG_TAG, "Connected to a device, address: %s", std::string(pClient->getPeerAddress()).c_str());
  if (!pClient->secureConnection(true)) {  // async = true
    BLEGC_LOGE(LOG_TAG, "Failed to initiate secure connection, address: %s",
               std::string(pClient->getPeerAddress()).c_str());
    _controllerRegistry._sendClientEvent({pClient->getPeerAddress(), BLEClientBondingFailed});
  }
}

void BLEControllerRegistry::ClientCallbacks::onConnectFail(NimBLEClient* pClient, int reason) {
  BLEGC_LOGE(LOG_TAG, "Failed connecting to a device, address: %s, reason: 0x%04x %s",
             std::string(pClient->getPeerAddress()).c_str(), reason, NimBLEUtils::returnCodeToString(reason));
  _controllerRegistry._sendClientEvent({pClient->getPeerAddress(), BLEClientConnectingFailed});
}

void BLEControllerRegistry::ClientCallbacks::onAuthenticationComplete(NimBLEConnInfo& connInfo) {
  if (connInfo.isBonded()) {
    BLEGC_LOGI(LOG_TAG, "Bonded successfully with a device, address: %s", std::string(connInfo.getAddress()).c_str());
    _controllerRegistry._sendClientEvent({connInfo.getAddress(), BLEClientBonded});
  } else {
    BLEGC_LOGW(LOG_TAG, "Failed to bond with a device, address: %s", std::string(connInfo.getAddress()).c_str());
    _controllerRegistry._sendClientEvent({connInfo.getAddress(), BLEClientBondingFailed});
  }
}

void BLEControllerRegistry::ClientCallbacks::onDisconnect(NimBLEClient* pClient, int reason) {
  BLEGC_LOGI(LOG_TAG, "Device disconnected, address: %s, reason: 0x%04x %s",
             std::string(pClient->getPeerAddress()).c_str(), reason, NimBLEUtils::returnCodeToString(reason));
  _controllerRegistry._sendClientEvent({pClient->getPeerAddress(), BLEClientDisconnected});
}
