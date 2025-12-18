#pragma once

#include <NimBLEAddress.h>
#include <string>
#include "BLEBaseController.h"
#include "BLEValueReceiver.h"
#include "BLEValueWriter.h"
#include "XboxBatteryState.h"
#include "XboxControlsState.h"
#include "XboxVibrationsCommand.h"

class XboxController final : public BLEBaseController<XboxController>,
                             public BLEValueReceiver<XboxControlsState>,
                             public BLEValueReceiver<XboxBatteryState>,
                             public BLEValueWriter<XboxVibrationsCommand> {
 public:
  XboxController();
  ~XboxController();

  using BLEValueReceiver<XboxControlsState>::read;
  using BLEValueReceiver<XboxControlsState>::onValueChanged;
  using BLEValueReceiver<XboxBatteryState>::read;
  using BLEValueReceiver<XboxBatteryState>::onValueChanged;

 protected:
  bool isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) override;
  bool init() override;
  bool deinit() override;
};
