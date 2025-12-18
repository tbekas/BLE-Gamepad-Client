#pragma once

#include <NimBLEAddress.h>
#include <string>
#include "BLEBaseController.h"
#include "BLEValueReceiver.h"
#include "SteamControlsState.h"

class SteamController;

class SteamController final : public BLEBaseController<SteamController>, public BLEValueReceiver<SteamControlsState> {
 public:
  SteamController();
  ~SteamController() override = default;

 protected:
  bool isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) override;
  bool init() override;
  bool deinit() override;
};
