#pragma once

#include <deque>
#include <map>

#include "BLEControllerAdapter.h"

#define CTRL_ADAPTER_MATCH_TYPE uint64_t
#define MAX_CTRL_ADAPTER_COUNT sizeof(CTRL_ADAPTER_MATCH_TYPE)

class BLEControllerAdapterRegistry {
 public:
  BLEControllerAdapterRegistry() = default;
  ~BLEControllerAdapterRegistry() = default;

  bool init();
  bool deinit();
  bool isInitialized() const;

  CTRL_ADAPTER_MATCH_TYPE matchAdapters(const NimBLEAdvertisedDevice* pAdvertisedDevice);
  CTRL_ADAPTER_MATCH_TYPE getMatchedAdapters(NimBLEAddress address);
  bool addAdapter(const BLEControllerAdapter& adapter);
  BLEControllerAdapter& getAdapter(unsigned int index);
  unsigned int getAdapterCount() const;

 private:
  bool _initialized{};
  std::deque<BLEControllerAdapter> _adapters{};
  std::map<NimBLEAddress, CTRL_ADAPTER_MATCH_TYPE> _matchedAdapters{};
};
