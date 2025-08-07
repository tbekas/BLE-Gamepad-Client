#include "BLEScanCallbacksImpl.h"
#include <bitset>
#include "BLEClientCallbacksImpl.h"
#include "BLEControllerRegistry.h"
#include "config.h"
#include "logger.h"

static auto* LOG_TAG = "BLEScanCallbacksImpl";

BLEClientCallbacksImpl clientCallbacks;

BLEScanCallbacksImpl::BLEScanCallbacksImpl(BLEAdapterRegistry& adapterRegistry,
                                           BLEControllerRegistry& controllerRegistry)
    : _adapterRegistry(adapterRegistry), _controllerRegistry(controllerRegistry) {}

void BLEScanCallbacksImpl::onResult(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  BLEGC_LOGD(LOG_TAG, "Device discovered, address: %s, address type: %d, name: %s",
             std::string(pAdvertisedDevice->getAddress()).c_str(), pAdvertisedDevice->getAddressType(),
             pAdvertisedDevice->getName().c_str());

  if (!_adapterRegistry.matchAdapters(pAdvertisedDevice)) {
    BLEGC_LOGD(LOG_TAG, "No adapters found for a device");
    return;
  }

  if (!BLEControllerRegistry::reserveController(pAdvertisedDevice->getAddress())) {
    return;
  }

  auto* pClient = NimBLEDevice::getClientByPeerAddress(pAdvertisedDevice->getAddress());
  if (pClient) {
    BLEGC_LOGD(LOG_TAG, "Reusing existing client for a device, address: %s",
               std::string(pClient->getPeerAddress()).c_str());
  } else {
    pClient = NimBLEDevice::createClient(pAdvertisedDevice->getAddress());
    if (!pClient) {
      BLEGC_LOGE(LOG_TAG, "Failed to create client for a device, address: %s",
                 std::string(pAdvertisedDevice->getAddress()).c_str());
      BLEControllerRegistry::releaseController(pAdvertisedDevice->getAddress());
      BLEControllerRegistry::_autoScanCheck();
      return;
    }
    pClient->setConnectTimeout(CONFIG_BT_BLEGC_CONN_TIMEOUT_MS);
    pClient->setClientCallbacks(&clientCallbacks, false);
  }

  BLEGC_LOGI(LOG_TAG, "Attempting to connect to a device, address: %s", std::string(pClient->getPeerAddress()).c_str());

  if (!pClient->connect(true, true, true)) {
    BLEGC_LOGE(LOG_TAG, "Failed to initiate connection, address: %s", std::string(pClient->getPeerAddress()).c_str());
    NimBLEDevice::deleteClient(pClient);
    BLEControllerRegistry::releaseController(pClient->getPeerAddress());
    BLEControllerRegistry::_autoScanCheck();
    return;
  }
}

void BLEScanCallbacksImpl::onScanEnd(const NimBLEScanResults& results, int reason) {
  BLEGC_LOGD(LOG_TAG, "Scan ended, reason: 0x%04x %s", reason, NimBLEUtils::returnCodeToString(reason));
  BLEControllerRegistry::_autoScanCheck();
}
