#pragma once

#include <NimBLEAddress.h>
#include <string>
#include "BLEBaseController.h"
#include "BLEValueReceiver.h"
#include "BLEValueWriter.h"
#include "XboxBatteryEvent.h"
#include "XboxControlsEvent.h"
#include "XboxVibrationsCommand.h"

class XboxController final : public BLEBaseController<XboxController>,
                             public BLEValueReceiver<XboxControlsEvent>,
                             public BLEValueReceiver<XboxBatteryEvent>,
                             public BLEValueWriter<XboxVibrationsCommand> {
 public:
  XboxController();
  ~XboxController();

  using BLEValueReceiver<XboxControlsEvent>::read;
  using BLEValueReceiver<XboxControlsEvent>::onValueChanged;
  using BLEValueReceiver<XboxBatteryEvent>::read;
  using BLEValueReceiver<XboxBatteryEvent>::onValueChanged;

 protected:
  bool isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) override;
  bool init(NimBLEClient* pClient) override;
  bool deinit() override;
};
