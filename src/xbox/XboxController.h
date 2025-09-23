#pragma once

#include <functional>
#include <string>
#include <NimBLEAddress.h>
#include "BLEBaseController.h"
#include "BLEValueReceiver.h"
#include "BLEValueWriter.h"
#include "XboxControlsEvent.h"
#include "XboxBatteryEvent.h"
#include "XboxVibrationsCommand.h"
#include "BLEControlsAPI.h"
#include "BLEBatteryAPI.h"

class XboxController final : public BLEBaseController, public BLEControlsAPI<XboxControlsEvent>, public BLEBatteryAPI<XboxBatteryEvent> {
 public:
  explicit XboxController(NimBLEAddress allowedAddress);
  explicit XboxController(const std::string& allowedAddress);
  XboxController();
  ~XboxController() = default;

  void writeVibrations(const XboxVibrationsCommand& cmd);
  void onConnect(const std::function<void(XboxController& c)>& callback);
  void onDisconnect(const std::function<void(XboxController& c)>& callback);

 protected:
  bool isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) override;
  bool init(NimBLEClient* pClient) override;
  bool deinit() override;

 private:
  BLEValueWriter<XboxVibrationsCommand> _vibrations;
};
