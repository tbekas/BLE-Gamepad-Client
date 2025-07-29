#include "BLEClientCallbacksImpl.h"
#include <NimBLEDevice.h>
#include "BLEControllerRegistry.h"
#include "logger.h"

void BLEClientCallbacksImpl::onConnect(NimBLEClient* pClient) {
  BLEGC_LOGD("Connected to a device, address: %s", std::string(pClient->getPeerAddress()).c_str());
  if (!pClient->secureConnection(true)) {  // async = true
    BLEGC_LOGE("Failed to initiate secure connection, address: %s", std::string(pClient->getPeerAddress()).c_str());
    // TODO: disconnect and tmp ban?
  }
}

void BLEClientCallbacksImpl::onConnectFail(NimBLEClient* pClient, int reason) {
  BLEGC_LOGE("Failed connecting to a device, address: %s, reason: 0x%04x %s",
             std::string(pClient->getPeerAddress()).c_str(), reason, NimBLEUtils::returnCodeToString(reason));
  NimBLEDevice::deleteClient(pClient);
  BLEControllerRegistry::_releaseController(pClient->getPeerAddress());
  BLEControllerRegistry::_autoScanCheck();
}

void BLEClientCallbacksImpl::onAuthenticationComplete(NimBLEConnInfo& connInfo) {
  if (connInfo.isBonded()) {
    BLEGC_LOGI("Bonded successfully with a device, address: %s", std::string(connInfo.getAddress()).c_str());
    BLEClientStatus msg = {connInfo.getAddress(), BLEClientConnected};
    if (xQueueSend(BLEControllerRegistry::_clientStatusQueue, &msg, 0) != pdPASS) {
      BLEGC_LOGE("Failed to send client status message");
    }
  } else {
    BLEGC_LOGW("Failed to bond with a device, address: %s", std::string(connInfo.getAddress()).c_str());
    // TODO: disconnect and tmp ban?
  }
  BLEControllerRegistry::_autoScanCheck();
}

void BLEClientCallbacksImpl::onDisconnect(NimBLEClient* pClient, int reason) {
  BLEGC_LOGI("Device disconnected, address: %s, reason: 0x%04x %s", std::string(pClient->getPeerAddress()).c_str(),
             reason, NimBLEUtils::returnCodeToString(reason));
  BLEClientStatus msg = {pClient->getPeerAddress(), BLEClientDisconnected};
  if (xQueueSend(BLEControllerRegistry::_clientStatusQueue, &msg, 0) != pdPASS) {
    BLEGC_LOGE("Failed to send client status message");
  }
}
