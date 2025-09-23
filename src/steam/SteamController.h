#pragma once

#include <functional>
#include <string>
#include <NimBLEAddress.h>
#include "BLEBaseController.h"
#include "BLEValueReceiver.h"
#include "SteamControlsEvent.h"

class SteamController;

class SteamController final : public BLEBaseController {
 public:
  explicit SteamController(NimBLEAddress allowedAddress);
  explicit SteamController(const std::string& allowedAddress);
  SteamController();
  ~SteamController() override = default;

  void readControls(SteamControlsEvent& event);
  void onControlsUpdate(const std::function<void(SteamControlsEvent& e)>& callback);

  void onConnect(const std::function<void(SteamController& c)>& callback);
  void onDisconnect(const std::function<void(SteamController& c)>& callback);

protected:
  bool isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) override;
  bool init(NimBLEClient* pClient) override;
  bool deinit() override;

 private:
  BLEValueReceiver<SteamControlsEvent> _controls;
};
