#include "BLEAdapterRegistry.h"

#include <bitset>
#include "logger.h"
#include "xbox.h"

static auto* LOG_TAG = "BLEAdapterRegistry";

CTRL_ADAPTER_MATCH_TYPE BLEAdapterRegistry::matchAdapters(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  auto configMatch = std::bitset<MAX_CTRL_ADAPTER_COUNT>();

  for (int i = 0; i < _adapters.size(); i++) {
    auto& config = _adapters[i];

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

  CTRL_ADAPTER_MATCH_TYPE value = configMatch.to_ullong();
  _cache[pAdvertisedDevice->getAddress()] = value;
  return value;
}

CTRL_ADAPTER_MATCH_TYPE BLEAdapterRegistry::getMatchedAdapters(NimBLEAddress address) {
  if (_cache.contains(address)) {
    return _cache[address];
  }

  return 0;
}

/**
 * @brief Registers an adapter for a new controller type. Adapter is used to set up a connection and to
 * decode raw data coming from the controller.
 * @param adapter Adapter to be added.
 * @return True if successful.
 */
bool BLEAdapterRegistry::addAdapter(const BLEControllerAdapter& adapter) {
  if (_adapters.size() >= MAX_CTRL_ADAPTER_COUNT) {
    BLEGC_LOGE(LOG_TAG, "Reached maximum number of adapters: %d", MAX_CTRL_ADAPTER_COUNT);
    return false;
  }
  if (adapter.controls.isDisabled() && adapter.battery.isDisabled() && adapter.vibrations.isDisabled()) {
    BLEGC_LOGE(LOG_TAG, "Invalid adapter, at least one of [`controls`, `battery`, `vibrations`] has to be enabled");
    return false;
  }

  _adapters.push_back(adapter);
  return true;
}
bool BLEAdapterRegistry::init() {
  // default adapters - lowest priority in front
  _adapters.push_front(blegc::xboxControllerAdapter);

  return true;
}
