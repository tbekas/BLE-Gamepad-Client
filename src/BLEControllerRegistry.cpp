#include "BLEControllerRegistry.h"

#include <BLEAutoScan.h>
#include <NimBLEClient.h>
#include <NimBLEDevice.h>
#include <NimBLEScan.h>
#include <NimBLEUtils.h>
#include <algorithm>
#include <optional>
#include <bitset>
#include <memory>
#include "BLEBaseController.h"
#include "config.h"
#include "logger.h"

BLEControllerRegistry::BLEClientEvent::operator std::string() const {
  std::string kindStr;
  // clang-format off
  switch (kind) {
    case BLEClientEventKind::BLEClientConnected: kindStr = "BLEClientConnected"; break;
    case BLEClientEventKind::BLEClientBonded: kindStr = "BLEClientBonded"; break;
    case BLEClientEventKind::BLEClientDisconnected: kindStr = "BLEClientDisconnected"; break;
    case BLEClientEventKind::BLEClientConnectingFailed: kindStr = "BLEClientConnectingFailed"; break;
    case BLEClientEventKind::BLEClientBondingFailed: kindStr = "BLEClientBondingFailed"; break;
  }
  // clang-format on
  return "BLEClientEvent address: " + std::string(address) + ", kind: " + kindStr;
}

BLEControllerRegistry::BLEControllerRegistry(TaskHandle_t& autoScanTask, TaskHandle_t& scanCallbackTask)
    : _startStopScanTask(autoScanTask),
      _scanCallbackTask(scanCallbackTask),
      _callbackTask(nullptr),
      _clientEventQueue(nullptr),
      _clientEventConsumerTask(nullptr),
      _controllers(new std::vector<BLEAbstractController*>()),
      _clientCallbacks(*this) {
  xTaskCreate(_callbackTaskFn, "_callbackTask", 10000, this, 0, &_callbackTask);
  configASSERT(_callbackTask);

  _clientEventQueue = xQueueCreate(10, sizeof(BLEClientEvent));
  configASSERT(_clientEventQueue);

  xTaskCreatePinnedToCore(_clientEventConsumerFn, "_clientEventConsumerTask", 10000, this, 0, &_clientEventConsumerTask,
                          CONFIG_BT_NIMBLE_PINNED_TO_CORE);
  configASSERT(_clientEventConsumerTask);
}

BLEControllerRegistry::~BLEControllerRegistry() {
  if (_clientEventConsumerTask != nullptr) {
    vTaskDelete(_clientEventConsumerTask);
  }
  if (_clientEventQueue != nullptr) {
    vQueueDelete(_clientEventQueue);
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
        BLEGC_LOGD("Controller already registered");
        delete pControllersNew;
        return;
      }
    }
    pControllersNew->push_back(pCtrl);
  } while (
      !_controllers.compare_exchange_weak(pControllersOld,
                                          pControllersNew));  // this loads updated data into controllersOld on failure

  delete pControllersOld;

  _notifyAutoScan();
  BLEGC_LOGD("Controller registered");
}

void BLEControllerRegistry::deregisterController(BLEAbstractController* pCtrl, bool notifyAutoScan) {
  configASSERT(pCtrl->isPendingDeregistration());

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
      BLEGC_LOGD("Controller not registered");
      pCtrl->markCompletedDeregistration();
      delete pControllersNew;
      return;
    }
  } while (!_controllers.compare_exchange_weak(pControllersOld,
                                               pControllersNew));  // this loads updated data into controllersOld

  delete pControllersOld;

  if (notifyAutoScan) {
    _notifyAutoScan();
  }

  pCtrl->markCompletedDeregistration();
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
    _sendClientEvent({address, BLEClientEventKind::BLEClientConnectingFailed});
    return;
  }

  pClient->setSelfDelete(false, false);
  pClient->setConnectTimeout(CONFIG_BT_BLEGC_CONN_TIMEOUT_MS);
  pClient->setClientCallbacks(&_clientCallbacks, false);

  pCtrl->setClient(pClient);

  BLEGC_LOGI("Attempting to connect to a device, address: %s", std::string(pClient->getPeerAddress()).c_str());

  // Calling pClient->connect() implicitly stops scan, so we need to invoke callback
  _runScanCallback();
  if (!pClient->connect(true, true, true)) {
    BLEGC_LOGE("Failed to initiate connection, address: %s", std::string(pClient->getPeerAddress()).c_str());
    _sendClientEvent({address, BLEClientEventKind::BLEClientConnectingFailed});
  }
}
BLEControllerRegistry::BLEControllerAllocationInfo BLEControllerRegistry::getControllerAllocationInfo() const {
  BLEControllerAllocationInfo result;
  for (auto* pCtrl : *_controllers.load()) {
    if (pCtrl->isAllocated()) {
      result.allocated++;
    } else {
      result.notAllocated++;
    }
  }
  return result;
}

BLEAbstractController* BLEControllerRegistry::_findController(const NimBLEAddress address) const {
  for (auto* pCtrl : *_controllers.load()) {
    if (pCtrl->getAddress() == address) {
      return pCtrl;
    }
  }

  return nullptr;
}

BLEAbstractController* BLEControllerRegistry::_findAndAllocateController(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  const auto address = pAdvertisedDevice->getAddress();

  std::vector<BLEAbstractController*> suitableControllers;

  for (auto* pCtrl : *_controllers.load()) {
    if (pCtrl->isAllocated() || !pCtrl->isSupported(pAdvertisedDevice) || pCtrl->isPendingDeregistration()) {
      continue;
    }
    suitableControllers.push_back(pCtrl);
  }

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

void BLEControllerRegistry::_sendClientEvent(const BLEClientEvent& msg) const {
  if (xQueueSend(_clientEventQueue, &msg, 0) != pdPASS) {
    BLEGC_LOGE("Failed to send client event message");
  }
}

void BLEControllerRegistry::_runCtrlCallback(const BLEAbstractController* pCtrl) const {
  xTaskNotify(_callbackTask, reinterpret_cast<uint32_t>(pCtrl), eSetValueWithOverwrite);
}

void BLEControllerRegistry::_notifyAutoScan() const {
  xTaskNotify(_startStopScanTask, static_cast<uint8_t>(BLEAutoScan::BLEAutoScanNotification::Auto), eSetValueWithOverwrite);
}

void BLEControllerRegistry::_runScanCallback() const {
  xTaskNotify(_scanCallbackTask, 0, eSetValueWithOverwrite);
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
      case BLEClientEventKind::BLEClientConnected: {
        if (!pCtrl->getClient()->secureConnection(true)) {  // async = true
          BLEGC_LOGE("Failed to initiate secure connection, address: %s", std::string(msg.address).c_str());
          pCtrl->getClient()->disconnect();
        }
        break;
      }
      case BLEClientEventKind::BLEClientBonded: {
        if (pCtrl->isPendingDeregistration()) {
          pCtrl->getClient()->disconnect();
          break;
        }

        auto retryCount = 2;
        while (!(pCtrl->hidInit(pCtrl->getClient()) && pCtrl->init(pCtrl->getClient())) && --retryCount) {
        }

        if (retryCount == 0) {
          BLEGC_LOGW("Failed to initialize controller, address: %s", std::string(msg.address).c_str());
          pCtrl->getClient()->disconnect();
          break;
        }

        pCtrl->markConnected();
        self->_runCtrlCallback(pCtrl);

        BLEGC_LOGD("Controller successfully initialized");
        self->_notifyAutoScan();
        break;
      }
      case BLEClientEventKind::BLEClientDisconnected: {
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
          self->_runCtrlCallback(pCtrl);
        }

        if (pCtrl->isPendingDeregistration()) {
          self->deregisterController(pCtrl, false);
        }

        self->_notifyAutoScan();
        break;
      }
      case BLEClientEventKind::BLEClientConnectingFailed: {
        if (!pCtrl->tryDeallocate()) {
          BLEGC_LOGE("Failed to deallocate controller %s", std::string(msg.address).c_str());
          break;
        }

        if (pCtrl->getClient() != nullptr && !NimBLEDevice::deleteClient(pCtrl->getClient())) {
          BLEGC_LOGE("Failed to delete client %s", std::string(msg.address).c_str());
        }

        pCtrl->setClient(nullptr);

        if (pCtrl->isPendingDeregistration()) {
          self->deregisterController(pCtrl, false);
        }

        self->_notifyAutoScan();
        break;
      }
      case BLEClientEventKind::BLEClientBondingFailed: {
        pCtrl->getClient()->disconnect();
        break;
      }
    }
  }
}

BLEControllerRegistry::ClientCallbacks::ClientCallbacks(BLEControllerRegistry& controllerRegistry)
    : _controllerRegistry(controllerRegistry) {}

void BLEControllerRegistry::ClientCallbacks::onConnect(NimBLEClient* pClient) {
  BLEGC_LOGD("Connected to a device, address: %s", std::string(pClient->getPeerAddress()).c_str());
  _controllerRegistry._sendClientEvent({pClient->getPeerAddress(), BLEClientEventKind::BLEClientConnected});
}

void BLEControllerRegistry::ClientCallbacks::onConnectFail(NimBLEClient* pClient, int reason) {
  BLEGC_LOGE("Failed connecting to a device, address: %s, reason: 0x%04x %s",
             std::string(pClient->getPeerAddress()).c_str(), reason, NimBLEUtils::returnCodeToString(reason));
  _controllerRegistry._sendClientEvent({pClient->getPeerAddress(), BLEClientEventKind::BLEClientConnectingFailed});
}

void BLEControllerRegistry::ClientCallbacks::onAuthenticationComplete(NimBLEConnInfo& connInfo) {
  if (connInfo.isBonded()) {
    BLEGC_LOGI("Bonded successfully with a device, address: %s", std::string(connInfo.getAddress()).c_str());
    _controllerRegistry._sendClientEvent({connInfo.getAddress(), BLEClientEventKind::BLEClientBonded});
  } else {
    BLEGC_LOGW("Failed to bond with a device, address: %s", std::string(connInfo.getAddress()).c_str());
    _controllerRegistry._sendClientEvent({connInfo.getAddress(), BLEClientEventKind::BLEClientBondingFailed});
  }
}

void BLEControllerRegistry::ClientCallbacks::onDisconnect(NimBLEClient* pClient, int reason) {
  BLEGC_LOGI("Device disconnected, address: %s, reason: 0x%04x %s", std::string(pClient->getPeerAddress()).c_str(),
             reason, NimBLEUtils::returnCodeToString(reason));
  _controllerRegistry._sendClientEvent({pClient->getPeerAddress(), BLEClientEventKind::BLEClientDisconnected});
}
