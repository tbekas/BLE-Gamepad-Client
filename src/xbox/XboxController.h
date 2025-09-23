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

class XboxController final : public BLEBaseController {
 public:
  explicit XboxController(NimBLEAddress allowedAddress);
  explicit XboxController(const std::string& allowedAddress);
  XboxController();
  ~XboxController() = default;

  void readControls(XboxControlsEvent& event);
  void onControlsUpdate(const std::function<void(XboxControlsEvent& e)>& callback);

  void readBattery(XboxBatteryEvent& event);
  void onBatteryUpdate(const std::function<void(XboxBatteryEvent& e)>& callback);
  void writeVibrations(const XboxVibrationsCommand& cmd);
  void onConnect(const std::function<void(XboxController& c)>& callback);
  void onDisconnect(const std::function<void(XboxController& c)>& callback);

 protected:
  bool isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) override;
  bool init(NimBLEClient* pClient) override;
  bool deinit() override;

 private:
  BLEValueReceiver<XboxControlsEvent> _controls;
  BLEValueReceiver<XboxBatteryEvent> _battery;
  BLEValueWriter<XboxVibrationsCommand> _vibrations;
};
