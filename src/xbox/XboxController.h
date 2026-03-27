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
  explicit XboxController(const NimBLEAddress& allowedAddress);
  explicit XboxController(const std::string& allowedAddress);
  ~XboxController() override;

  using BLEValueReceiver<XboxControlsState>::read;
  using BLEValueReceiver<XboxControlsState>::onValueChanged;
  using BLEValueReceiver<XboxBatteryState>::read;
  using BLEValueReceiver<XboxBatteryState>::onValueChanged;

 protected:
  bool isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) override;
  bool init() override;
  bool deinit() override;
};
