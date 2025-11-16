#pragma once

#include <NimBLEAddress.h>
#include <string>
#include "BLEBaseController.h"
#include "BLEValueReceiver.h"
#include "SteamControlsEvent.h"

class SteamController;

class SteamController final : public BLEBaseController<SteamController>, public BLEValueReceiver<SteamControlsEvent> {
 public:
  SteamController();
  ~SteamController() override = default;

 protected:
  bool isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) override;
  bool init(NimBLEClient* pClient) override;
  bool deinit() override;
};
