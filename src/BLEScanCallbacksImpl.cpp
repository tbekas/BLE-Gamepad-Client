#include "BLEScanCallbacksImpl.h"
#include "logger.h"
#include "BLEControllerRegistry.h"
#include "config.h"
#include "BLEClientCallbacksImpl.h"
#include <bitset>

BLEClientCallbacksImpl clientCallbacks;

void BLEScanCallbacksImpl::onResult(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
    BLEGC_LOGD("Device discovered, address: %s, address type: %d, name: %s",
               std::string(pAdvertisedDevice->getAddress()).c_str(), pAdvertisedDevice->getAddressType(),
               pAdvertisedDevice->getName().c_str());

    auto configMatch = std::bitset<MAX_CTRL_ADAPTER_COUNT>();

    for (int i = 0; i < BLEControllerRegistry::_adapters.size(); i++) {
      auto& config = BLEControllerRegistry::_adapters[i];

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

    if (!BLEControllerRegistry::_reserveController(pAdvertisedDevice->getAddress())) {
      return;
    }

    BLEControllerRegistry::_adapterMatch[pAdvertisedDevice->getAddress()] = configMatch.to_ulong();

    auto* pClient = NimBLEDevice::getClientByPeerAddress(pAdvertisedDevice->getAddress());
    if (pClient) {
      BLEGC_LOGD("Reusing existing client for a device, address: %s", std::string(pClient->getPeerAddress()).c_str());
    } else {
      pClient = NimBLEDevice::createClient(pAdvertisedDevice->getAddress());
      if (!pClient) {
        BLEGC_LOGE("Failed to create client for a device, address: %s",
                   std::string(pAdvertisedDevice->getAddress()).c_str());
        BLEControllerRegistry::_releaseController(pAdvertisedDevice->getAddress());
        BLEControllerRegistry::_autoScanCheck();
        return;
      }
      pClient->setConnectTimeout(CONFIG_BT_BLEGC_CONN_TIMEOUT_MS);
      pClient->setClientCallbacks(&clientCallbacks, false);
    }

    BLEGC_LOGI("Attempting to connect to a device, address: %s", std::string(pClient->getPeerAddress()).c_str());

    if (!pClient->connect(true, true, false)) {
      BLEGC_LOGE("Failed to initiate connection, address: %s", std::string(pClient->getPeerAddress()).c_str());
      NimBLEDevice::deleteClient(pClient);
      BLEControllerRegistry::_releaseController(pClient->getPeerAddress());
      BLEControllerRegistry::_autoScanCheck();
      return;
    }
  }

  void BLEScanCallbacksImpl::onScanEnd(const NimBLEScanResults& results, int reason) {
    BLEGC_LOGD("Scan ended, reason: 0x%04x %s", reason, NimBLEUtils::returnCodeToString(reason));
    BLEControllerRegistry::_autoScanCheck();
  }
