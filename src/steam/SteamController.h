#pragma once

#include <NimBLEAddress.h>
#include "BLEBaseController.h"
#include "BLENotifiableSignal.h"
#include "SteamControlsEvent.h"

class SteamController final : public BLEBaseController {
 public:
  explicit SteamController(NimBLEAddress allowedAddress);
  explicit SteamController(const std::string& allowedAddress);
  SteamController();
  ~SteamController() override = default;

  void readControls(SteamControlsEvent& event);

protected:
  bool isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) override;
  bool init(NimBLEClient* pClient) override;
  bool deinit() override;

 private:
  BLENotifiableSignal<SteamControlsEvent> _controls;
};
