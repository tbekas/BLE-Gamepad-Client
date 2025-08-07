
#include "BLEScanCallbacksImpl.h"

class BLEAutoScan {
 public:
  explicit BLEAutoScan(BLEScanCallbacksImpl& scanCallbacks);
  ~BLEAutoScan() = default;

  void enable();
  void disable();
  bool isEnabled() const;

  bool init();
  bool isInitialized() const;

  void trigger();

 private:
  bool _enabled = true;
  bool _initialized = false;
  BLEScanCallbacksImpl& _scanCallbacks;
};
