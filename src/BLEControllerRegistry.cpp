#include "BLEControllerRegistry.h"

#include <NimBLEClient.h>
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEUtils.h>
#include <bitset>
#include <memory>
#include <optional>
#include "BLEAbstractController.h"
#include "config.h"
#include "logger.h"
#include "messages.h"

BLEControllerRegistry::ClientEvent::operator std::string() const {
  std::string kindStr;
  // clang-format off
  switch (kind) {
    case ClientEventKind::ClientConnected: kindStr = "BLEClientConnected"; break;
    case ClientEventKind::ClientBonded: kindStr = "BLEClientBonded"; break;
    case ClientEventKind::ClientDisconnected: kindStr = "BLEClientDisconnected"; break;
    case ClientEventKind::ClientConnectionFailed: kindStr = "BLEClientConnectingFailed"; break;
    case ClientEventKind::ClientBondingFailed: kindStr = "BLEClientBondingFailed"; break;
  }
  // clang-format on
  return "BLEClientEvent address: " + std::string(address) + ", kind: " + kindStr;
}

BLEControllerRegistry::BLEControllerRegistry(TaskHandle_t& autoScanTask, QueueHandle_t& userCallbackQueue)
    : _controllers({}),
      _controllersMutex(nullptr),
      _autoScanTask(autoScanTask),
      _userCallbackQueue(userCallbackQueue),
      _clientEventQueue(nullptr),
      _clientEventConsumerTask(nullptr),
      _clientCallbacksImpl(*this) {
  _controllersMutex = xSemaphoreCreateMutex();
  configASSERT(_controllersMutex);

  _clientEventQueue = xQueueCreate(10, sizeof(ClientEvent));
  configASSERT(_clientEventQueue);

  xTaskCreatePinnedToCore(_clientEventConsumerFn, "_clientEventConsumerTask", 10000, this, 0, &_clientEventConsumerTask,
                          CONFIG_BT_NIMBLE_PINNED_TO_CORE);
  configASSERT(_clientEventConsumerTask);
}

BLEControllerRegistry::~BLEControllerRegistry() {
  if (_clientEventConsumerTask != nullptr) {
    vTaskDelete(_clientEventConsumerTask);
    _clientEventConsumerTask = nullptr;
  }
  if (_clientEventQueue != nullptr) {
    vQueueDelete(_clientEventQueue);
    _clientEventQueue = nullptr;
  }

  if (_controllersMutex != nullptr) {
    vSemaphoreDelete(_controllersMutex);
    _controllersMutex = nullptr;
  }

  _controllers.clear();
}

void BLEControllerRegistry::registerController(BLEAbstractController* pCtrl) {

  configASSERT(xSemaphoreTake(_controllersMutex, portMAX_DELAY));
  for (auto* pSomeCtrl: _controllers) {
    if (pSomeCtrl == pCtrl) {
      BLEGC_LOGD("Controller already registered");
      break;
    }
  }
  _controllers.push_back(pCtrl);
  configASSERT(xSemaphoreGive(_controllersMutex));

  BLEGC_LOGD("Controller registered");
  _notifyAutoScan();
}

void BLEControllerRegistry::deregisterController(BLEAbstractController* pCtrl, bool notifyAutoScan) {
  if (!pCtrl->isPendingDeregistration()) {
    BLEGC_LOGE("Controller not marked for deregistration");
    return;
  }

  auto* pClient = pCtrl->getClient();
  if (pClient && pClient->isConnected()) {
    pClient->disconnect();
    return;  // deregistration will continue after completing disconnect
  }

  if (pCtrl->isAllocated()) {
    // connect is likely in progress, we can try to cancel it
    if (pClient && !pClient->isConnected()) {
      if (pClient->cancelConnect()) {
        BLEGC_LOGD("Cancel connect command sent successfully");
      } else {
        BLEGC_LOGD("Cancel connect command failed");
      }
    }
    return;  // deregistration will continue after canceling connection
  }

  configASSERT(xSemaphoreTake(_controllersMutex, portMAX_DELAY));
  bool found = false;
  for (auto it = _controllers.begin(); it != _controllers.end();) {
    if (*it == pCtrl) {
      _controllers.erase(it);
      found = true;
      break;
    }
    ++it;
  }
  configASSERT(xSemaphoreGive(_controllersMutex));

  if (!found) {
    BLEGC_LOGD("Controller not registered");
  }

  pCtrl->markCompletedDeregistration();

  if (notifyAutoScan) {
    _notifyAutoScan();
  }

  BLEGC_LOGD("Controller deregistered");
}

void BLEControllerRegistry::tryConnectController(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  auto* pCtrl = _findAndAllocateController(pAdvertisedDevice);
  if (!pCtrl) {
    return;
  }

  const auto address = pAdvertisedDevice->getAddress();

  BLEGC_LOGD("Creating a client for a device, address: %s", std::string(address).c_str());
  auto* pClient = NimBLEDevice::createClient(address);

  if (!pClient) {
    BLEGC_LOGE("Failed to create client for a device, address: %s", std::string(address).c_str());
    _sendClientEvent({address, ClientEventKind::ClientConnectionFailed});
    return;
  }

  pClient->setSelfDelete(false, false);
  pClient->setConnectTimeout(CONFIG_BT_BLEGC_CONN_TIMEOUT_MS);
  pClient->setClientCallbacks(&_clientCallbacksImpl, false);

  pCtrl->setClient(pClient);

  BLEGC_LOGI("Attempting to connect to a device, address: %s", std::string(pClient->getPeerAddress()).c_str());

  if (!pClient->connect(true, true, true)) {
    BLEGC_LOGE("Failed to initiate connection, address: %s", std::string(pClient->getPeerAddress()).c_str());
    _sendClientEvent({address, ClientEventKind::ClientConnectionFailed});
    return;
  }

  pCtrl->markConnecting();
  _sendUserCallbackMsg({BLEUserCallbackKind::ScanStopped});  // pClient->connect() implicitly stopped a scan
  _sendUserCallbackMsg({BLEUserCallbackKind::ControllerConnecting, pCtrl});
}
BLEControllerRegistry::AllocationInfo BLEControllerRegistry::getAllocationInfo() const {
  AllocationInfo result;

  configASSERT(xSemaphoreTake(_controllersMutex, portMAX_DELAY));
  for (const auto* pCtrl : _controllers) {
    if (pCtrl->isAllocated()) {
      result.allocated++;
    } else {
      result.notAllocated++;
    }
  }
  configASSERT(xSemaphoreGive(_controllersMutex));

  return result;
}

BLEAbstractController* BLEControllerRegistry::_findController(const NimBLEAddress& address) const {
  BLEAbstractController* result = nullptr;
  configASSERT(xSemaphoreTake(_controllersMutex, portMAX_DELAY));
  for (auto* pCtrl : _controllers) {
    if (pCtrl->getAddress() == address) {
      result = pCtrl;
      break;
    }
  }
  configASSERT(xSemaphoreGive(_controllersMutex));

  return result;
}

BLEAbstractController* BLEControllerRegistry::_findAndAllocateController(
    const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  const auto address = pAdvertisedDevice->getAddress();

  std::vector<BLEAbstractController*> suitableControllers;

  configASSERT(xSemaphoreTake(_controllersMutex, portMAX_DELAY));
  for (auto* pCtrl : _controllers) {
    if (pCtrl->isAllocated() || !pCtrl->isSupported(pAdvertisedDevice) || pCtrl->isPendingDeregistration()) {
      continue;
    }
    suitableControllers.push_back(pCtrl);
  }
  configASSERT(xSemaphoreGive(_controllersMutex));

  // allocate ctrl allocated last time
  for (auto* pCtrl : suitableControllers) {
    if (!pCtrl->getLastAddress().isNull() && pCtrl->getLastAddress() == address) {
      if (pCtrl->tryAllocate(address)) {
        return pCtrl;
      }
    }
  }

  // allocate ctrl never allocated
  for (auto* pCtrl : suitableControllers) {
    if (pCtrl->getLastAddress().isNull()) {
      if (pCtrl->tryAllocate(address)) {
        return pCtrl;
      }
    }
  }

  // allocate any other controller (don't try to allocate controllers already tried)
  for (auto* pCtrl : suitableControllers) {
    if (!pCtrl->getLastAddress().isNull() && pCtrl->getLastAddress() != address) {
      if (pCtrl->tryAllocate(address)) {
        return pCtrl;
      }
    }
  }

  BLEGC_LOGD("No suitable controller found to allocate for a device, address %s", std::string(address).c_str());
  return nullptr;
}

void BLEControllerRegistry::_sendClientEvent(const ClientEvent& msg) const {
  if (xQueueSend(_clientEventQueue, &msg, 0) != pdPASS) {
    BLEGC_LOGE("Failed to send client event message");
  }
}

void BLEControllerRegistry::_sendUserCallbackMsg(const BLEUserCallback& msg) const {
  if (xQueueSend(_userCallbackQueue, &msg, 0) != pdPASS) {
    BLEGC_LOGE("Failed to send user callback message");
  }
}

void BLEControllerRegistry::_notifyAutoScan(const BLEAutoScanNotification notification) const {
  xTaskNotify(_autoScanTask, static_cast<uint8_t>(notification), eSetValueWithOverwrite);
}

void BLEControllerRegistry::_clientEventConsumerFn(void* pvParameters) {
  auto* self = static_cast<BLEControllerRegistry*>(pvParameters);

  while (true) {
    ClientEvent msg{};
    if (xQueueReceive(self->_clientEventQueue, &msg, portMAX_DELAY) != pdTRUE) {
      BLEGC_LOGE("Failed to receive client event message");
      return;
    }

    BLEGC_LOGD("Handling message %s", std::string(msg).c_str());

    auto* pCtrl = self->_findController(msg.address);
    if (!pCtrl) {
      BLEGC_LOGE("Controller not found, address: %s", std::string(msg.address).c_str());
      break;
    }

    switch (msg.kind) {
      case ClientEventKind::ClientConnected: {
        if (!pCtrl->getClient()->secureConnection(true)) {  // async = true
          BLEGC_LOGE("Failed to initiate secure connection, address: %s", std::string(msg.address).c_str());
          pCtrl->getClient()->disconnect();
        }
        break;
      }
      case ClientEventKind::ClientBonded: {
        if (pCtrl->isPendingDeregistration()) {
          pCtrl->getClient()->disconnect();
          break;
        }

        auto retryCount = 2;
        while (!(pCtrl->hidInit() && pCtrl->init()) && --retryCount) {
        }

        if (retryCount == 0) {
          BLEGC_LOGW("Failed to initialize controller, address: %s", std::string(msg.address).c_str());
          pCtrl->getClient()->disconnect();
          break;
        }

        pCtrl->markConnected();
        self->_sendUserCallbackMsg({BLEUserCallbackKind::ControllerConnected, pCtrl});

        BLEGC_LOGD("Controller successfully initialized");
        self->_notifyAutoScan();
        break;
      }
      case ClientEventKind::ClientDisconnected: {
        if (!pCtrl->deinit()) {
          BLEGC_LOGW("Failed to deinitialize controller, address: %s", std::string(msg.address).c_str());
        }

        if (!pCtrl->tryDeallocate()) {
          BLEGC_LOGE("Failed to deallocate controller %s", std::string(msg.address).c_str());
          break;
        }

        if (!NimBLEDevice::deleteClient(pCtrl->getClient())) {
          BLEGC_LOGE("Failed to delete client %s", std::string(msg.address).c_str());
        }

        pCtrl->setClient(nullptr);

        if (pCtrl->isConnected()) {
          pCtrl->markDisconnected();
          self->_sendUserCallbackMsg({BLEUserCallbackKind::ControllerDisconnected, pCtrl});
        } else if (pCtrl->isConnecting()) {
          pCtrl->markDisconnected();
          self->_sendUserCallbackMsg({BLEUserCallbackKind::ControllerConnectionFailed, pCtrl});
        }

        if (pCtrl->isPendingDeregistration()) {
          self->deregisterController(pCtrl, false);
        }

        self->_notifyAutoScan();
        break;
      }
      case ClientEventKind::ClientConnectionFailed: {
        if (!pCtrl->tryDeallocate()) {
          BLEGC_LOGE("Failed to deallocate controller %s", std::string(msg.address).c_str());
          break;
        }

        if (pCtrl->getClient() != nullptr && !NimBLEDevice::deleteClient(pCtrl->getClient())) {
          BLEGC_LOGE("Failed to delete client %s", std::string(msg.address).c_str());
        }

        pCtrl->setClient(nullptr);

        if (pCtrl->isConnecting()) {
          pCtrl->markDisconnected();
          self->_sendUserCallbackMsg({BLEUserCallbackKind::ControllerConnectionFailed, pCtrl});
        }

        if (pCtrl->isPendingDeregistration()) {
          self->deregisterController(pCtrl, false);
        }

        self->_notifyAutoScan();
        break;
      }
      case ClientEventKind::ClientBondingFailed: {
        pCtrl->getClient()->disconnect();
        break;
      }
    }
  }
}

BLEControllerRegistry::ClientCallbacksImpl::ClientCallbacksImpl(BLEControllerRegistry& controllerRegistry)
    : _controllerRegistry(controllerRegistry) {}

void BLEControllerRegistry::ClientCallbacksImpl::onConnect(NimBLEClient* pClient) {
  BLEGC_LOGD("Connected to a device, address: %s", std::string(pClient->getPeerAddress()).c_str());
  _controllerRegistry._sendClientEvent({pClient->getPeerAddress(), ClientEventKind::ClientConnected});
}

void BLEControllerRegistry::ClientCallbacksImpl::onConnectFail(NimBLEClient* pClient, int reason) {
  BLEGC_LOGE("Failed connecting to a device, address: %s, reason: 0x%04x %s",
             std::string(pClient->getPeerAddress()).c_str(), reason, NimBLEUtils::returnCodeToString(reason));
  _controllerRegistry._sendClientEvent({pClient->getPeerAddress(), ClientEventKind::ClientConnectionFailed});
}

void BLEControllerRegistry::ClientCallbacksImpl::onAuthenticationComplete(NimBLEConnInfo& connInfo) {
  if (connInfo.isBonded()) {
    BLEGC_LOGI("Bonded successfully with a device, address: %s", std::string(connInfo.getAddress()).c_str());
    _controllerRegistry._sendClientEvent({connInfo.getAddress(), ClientEventKind::ClientBonded});
  } else {
    BLEGC_LOGW("Failed to bond with a device, address: %s", std::string(connInfo.getAddress()).c_str());
    _controllerRegistry._sendClientEvent({connInfo.getAddress(), ClientEventKind::ClientBondingFailed});
  }
}

void BLEControllerRegistry::ClientCallbacksImpl::onDisconnect(NimBLEClient* pClient, int reason) {
  BLEGC_LOGI("Device disconnected, address: %s, reason: 0x%04x %s", std::string(pClient->getPeerAddress()).c_str(),
             reason, NimBLEUtils::returnCodeToString(reason));
  _controllerRegistry._sendClientEvent({pClient->getPeerAddress(), ClientEventKind::ClientDisconnected});
}
