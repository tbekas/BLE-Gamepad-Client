#pragma once

#include <deque>
#include <map>

#include "BLEControllerModel.h"

#define CTRL_MODEL_MATCH_TYPE uint64_t
#define MAX_CTRL_MODEL_COUNT sizeof(CTRL_MODEL_MATCH_TYPE) * 8

class BLEDeviceMatcher {
 public:
  BLEDeviceMatcher() = default;
  ~BLEDeviceMatcher() = default;

  bool init();
  bool deinit();
  bool isInitialized() const;

  CTRL_MODEL_MATCH_TYPE matchModels(const NimBLEAdvertisedDevice* pAdvertisedDevice);
  CTRL_MODEL_MATCH_TYPE getMatchedModels(NimBLEAddress address);
  bool addModel(const BLEControllerModel& model);
  BLEControllerModel& getModel(unsigned int index);
  unsigned int getModelCount() const;

 private:
  bool _initialized{};
  std::deque<BLEControllerModel> _models{};
  std::map<NimBLEAddress, CTRL_MODEL_MATCH_TYPE> _matchedModels{};
};
