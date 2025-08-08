#include "BLEControllerAdapterRegistry.h"

#include <bitset>
#include "logger.h"
#include "xbox.h"

static auto* LOG_TAG = "BLEAdapterRegistry";

bool BLEControllerAdapterRegistry::init() {
  if (_initialized) {
    return false;
  }

  // default adapters - lowest priority in front
  _adapters.push_front(blegc::xboxControllerAdapter);

  _initialized = true;
  return true;
}
bool BLEControllerAdapterRegistry::deinit() {
  if (!_initialized) {
    return false;
  }

  _adapters.clear();

  _initialized = false;
  return true;
}
bool BLEControllerAdapterRegistry::isInitialized() const {
  return _initialized;
}

CTRL_ADAPTER_MATCH_TYPE BLEControllerAdapterRegistry::matchAdapters(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  auto adapterMatch = std::bitset<MAX_CTRL_ADAPTER_COUNT>();

  for (int i = 0; i < _adapters.size(); i++) {
    auto& config = _adapters[i];

    if (pAdvertisedDevice->haveName() && !config.deviceName.empty() &&
        pAdvertisedDevice->getName() == config.deviceName) {
      adapterMatch[i] = true;
      continue;
    }

    if (config.controls.isEnabled() && pAdvertisedDevice->isAdvertisingService(config.controls.serviceUUID)) {
      adapterMatch[i] = true;
      continue;
    }

    if (config.battery.isEnabled() && pAdvertisedDevice->isAdvertisingService(config.battery.serviceUUID)) {
      adapterMatch[i] = true;
      continue;
    }

    if (config.vibrations.isEnabled() && pAdvertisedDevice->isAdvertisingService(config.vibrations.serviceUUID)) {
      adapterMatch[i] = true;
      continue;
    }
  }

  CTRL_ADAPTER_MATCH_TYPE value = adapterMatch.to_ullong();
  _matchedAdapters[pAdvertisedDevice->getAddress()] = value;
  return value;
}

CTRL_ADAPTER_MATCH_TYPE BLEControllerAdapterRegistry::getMatchedAdapters(NimBLEAddress address) {
  if (_matchedAdapters.contains(address)) {
    return _matchedAdapters[address];
  }

  return 0;
}

bool BLEControllerAdapterRegistry::addAdapter(const BLEControllerAdapter& adapter) {
  if (_adapters.size() >= MAX_CTRL_ADAPTER_COUNT) {
    BLEGC_LOGE(LOG_TAG, "Reached maximum number of adapters: %d", MAX_CTRL_ADAPTER_COUNT);
    return false;
  }
  if (!adapter.controls.isEnabled() && !adapter.battery.isEnabled() && !adapter.vibrations.isEnabled()) {
    BLEGC_LOGE(LOG_TAG, "Invalid adapter, at least one of [`controls`, `battery`, `vibrations`] has to be enabled");
    return false;
  }

  _adapters.push_back(adapter);
  return true;
}
BLEControllerAdapter& BLEControllerAdapterRegistry::getAdapter(const unsigned int index) {
  return _adapters[index];
}

unsigned int BLEControllerAdapterRegistry::getAdapterCount() const {
  return _adapters.size();
}
