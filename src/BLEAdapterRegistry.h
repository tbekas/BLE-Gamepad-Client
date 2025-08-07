#pragma once

#include <deque>
#include <map>

#include "BLEControllerAdapter.h"

#define CTRL_ADAPTER_MATCH_TYPE uint64_t
#define MAX_CTRL_ADAPTER_COUNT sizeof(CTRL_ADAPTER_MATCH_TYPE)

class BLEAdapterRegistry {

public:
  BLEAdapterRegistry() = default;
  ~BLEAdapterRegistry() = default;

  CTRL_ADAPTER_MATCH_TYPE matchAdapters(const NimBLEAdvertisedDevice* pAdvertisedDevice);
  CTRL_ADAPTER_MATCH_TYPE getMatchedAdapters(NimBLEAddress address);
  bool addAdapter(const BLEControllerAdapter& adapter);

  bool init();


private:
  std::deque<BLEControllerAdapter> _adapters{};
  std::map<NimBLEAddress, CTRL_ADAPTER_MATCH_TYPE> _cache{};
};
