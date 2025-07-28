#include "ScanCallbacks.h"
#include "logger.h"
#include "BLEGamepadClient.h"
#include "config.h"
#include "ClientCallbacks.h"
#include <bitset>

ClientCallbacks clientCallbacks;

void ScanCallbacks::onResult(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
    BLEGC_LOGD("Device discovered, address: %s, address type: %d, name: %s",
               std::string(pAdvertisedDevice->getAddress()).c_str(), pAdvertisedDevice->getAddressType(),
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

    auto* pClient = NimBLEDevice::getClientByPeerAddress(pAdvertisedDevice->getAddress());
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

  void ScanCallbacks::onScanEnd(const NimBLEScanResults& results, int reason) {
    BLEGC_LOGD("Scan ended, reason: 0x%04x %s", reason, NimBLEUtils::returnCodeToString(reason));
    BLEGamepadClient::_autoScanCheck();
  }
