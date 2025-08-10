#include "BLEControllerModelRegistry.h"

#include <bitset>
#include "logger.h"
#include "xbox.h"

static auto* LOG_TAG = "BLEModelRegistry";

bool BLEControllerModelRegistry::init() {
  if (_initialized) {
    return false;
  }

  // default models - lowest priority in front
  _models.push_front(blegc::xboxControllerModel);

  _initialized = true;
  return true;
}
bool BLEControllerModelRegistry::deinit() {
  if (!_initialized) {
    return false;
  }

  _models.clear();

  _initialized = false;
  return true;
}
bool BLEControllerModelRegistry::isInitialized() const {
  return _initialized;
}

CTRL_MODEL_MATCH_TYPE BLEControllerModelRegistry::matchModels(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  auto modelMatch = std::bitset<MAX_CTRL_MODEL_COUNT>();

  for (int i = 0; i < _models.size(); i++) {
    auto& config = _models[i];

    if (pAdvertisedDevice->haveName() && !config.advertisedName.empty() &&
        pAdvertisedDevice->getName() == config.advertisedName) {
      modelMatch[i] = true;
      continue;
    }

    if (config.controls.isEnabled() && pAdvertisedDevice->isAdvertisingService(config.controls.serviceUUID)) {
      modelMatch[i] = true;
      continue;
    }

    if (config.battery.isEnabled() && pAdvertisedDevice->isAdvertisingService(config.battery.serviceUUID)) {
      modelMatch[i] = true;
      continue;
    }

    if (config.vibrations.isEnabled() && pAdvertisedDevice->isAdvertisingService(config.vibrations.serviceUUID)) {
      modelMatch[i] = true;
      continue;
    }
  }

  CTRL_MODEL_MATCH_TYPE value = modelMatch.to_ullong();
  _matchedModels[pAdvertisedDevice->getAddress()] = value;
  return value;
}

CTRL_MODEL_MATCH_TYPE BLEControllerModelRegistry::getMatchedModels(NimBLEAddress address) {
  if (_matchedModels.contains(address)) {
    return _matchedModels[address];
  }

  return 0;
}

bool BLEControllerModelRegistry::addModel(const BLEControllerModel& model) {
  if (_models.size() >= MAX_CTRL_MODEL_COUNT) {
    BLEGC_LOGE(LOG_TAG, "Reached maximum number of models: %d", MAX_CTRL_MODEL_COUNT);
    return false;
  }
  if (!model.controls.isEnabled() && !model.battery.isEnabled() && !model.vibrations.isEnabled()) {
    BLEGC_LOGE(LOG_TAG, "Invalid model, at least one of [`controls`, `battery`, `vibrations`] has to be enabled");
    return false;
  }

  _models.push_back(model);
  return true;
}
BLEControllerModel& BLEControllerModelRegistry::getModel(const unsigned int index) {
  return _models[index];
}

unsigned int BLEControllerModelRegistry::getModelCount() const {
  return _models.size();
}
