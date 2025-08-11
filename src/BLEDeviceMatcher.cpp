#include "BLEDeviceMatcher.h"

#include <bitset>
#include "logger.h"
#include "xbox.h"
#include "steam.h"

static auto* LOG_TAG = "BLEDeviceMatcher";

bool BLEDeviceMatcher::init() {
  if (_initialized) {
    return false;
  }

  // default models - lowest priority in front
  _models.push_front(blegc::steamControllerModel);
  _models.push_front(blegc::xboxControllerModel);

  _initialized = true;
  return true;
}
bool BLEDeviceMatcher::deinit() {
  if (!_initialized) {
    return false;
  }

  _models.clear();

  _initialized = false;
  return true;
}
bool BLEDeviceMatcher::isInitialized() const {
  return _initialized;
}

CTRL_MODEL_MATCH_TYPE BLEDeviceMatcher::matchModels(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  auto modelMatch = std::bitset<MAX_CTRL_MODEL_COUNT>();

  for (int i = 0; i < _models.size(); i++) {
    auto& model = _models[i];

    if (!model.advertisedName.empty()) {
      modelMatch[i] = pAdvertisedDevice->haveName() && model.advertisedName == pAdvertisedDevice->getName();
      continue;
    }

    for (auto& spec : model.controls) {
      if (pAdvertisedDevice->isAdvertisingService(spec.serviceUUID)) {
        modelMatch[i] = true;
        break; // TODO break out of 2 loops
      }
    }

    for (auto& spec : model.battery) {
      if (pAdvertisedDevice->isAdvertisingService(spec.serviceUUID)) {
        modelMatch[i] = true;
        break; // TODO break out of 2 loops
      }
    }

    if (model.vibrations.isEnabled() && pAdvertisedDevice->isAdvertisingService(model.vibrations.serviceUUID)) {
      modelMatch[i] = true;
      continue;
    }
  }

  CTRL_MODEL_MATCH_TYPE value = modelMatch.to_ullong();
  _matchedModels[pAdvertisedDevice->getAddress()] = value;
  return value;
}

CTRL_MODEL_MATCH_TYPE BLEDeviceMatcher::getMatchedModels(const NimBLEAddress address) {
  return _matchedModels[address];
}

bool BLEDeviceMatcher::addModel(const BLEControllerModel& model) {
  if (_models.size() >= MAX_CTRL_MODEL_COUNT) {
    BLEGC_LOGE(LOG_TAG, "Reached maximum number of models: %d", MAX_CTRL_MODEL_COUNT);
    return false;
  }
  if (model.controls.empty() && model.battery.empty() && !model.vibrations.isEnabled()) {
    BLEGC_LOGE(LOG_TAG, "Invalid model, at least one of [`controls`, `battery`, `vibrations`] has to be defined");
    return false;
  }

  _models.push_back(model);
  return true;
}
BLEControllerModel& BLEDeviceMatcher::getModel(const unsigned int index) {
  return _models[index];
}

unsigned int BLEDeviceMatcher::getModelCount() const {
  return _models.size();
}
