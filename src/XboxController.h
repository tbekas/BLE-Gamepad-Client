#pragma once

#include <NimBLEAddress.h>
#include "BLEBaseController.h"
#include "XboxBatteryEvent.h"
#include "XboxControlsEvent.h"
#include "BLENotifiableSignal.h"
#include "BLEWritableSignal.h"

#include "XboxVibrationsCommand.h"

using OnControlsUpdate = std::function<void(XboxControlsEvent& e)>;
using OnBatteryUpdate = std::function<void(XboxBatteryEvent& e)>;

class XboxController final : public BLEBaseController {
 public:
  explicit XboxController(NimBLEAddress allowedAddress);
  explicit XboxController(const std::string& allowedAddress);
  XboxController();
  ~XboxController() = default;

  void readControls(XboxControlsEvent& event);
  void onControlsUpdate(const OnControlsUpdate& callback);
  void readBattery(XboxBatteryEvent& event);
  void onBatteryUpdate(const OnBatteryUpdate& callback);
  void writeVibrations(const XboxVibrationsCommand& cmd);

 protected:
  bool isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) override;
  bool init(NimBLEClient* pClient) override;
  bool deinit() override;

 private:
  BLENotifiableSignal<XboxControlsEvent> _controls;
  BLENotifiableSignal<XboxBatteryEvent> _battery;
  BLEWritableSignal<XboxVibrationsCommand> _vibrations;
};
