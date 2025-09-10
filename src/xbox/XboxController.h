#pragma once

#include <NimBLEAddress.h>
#include "BLEBaseController.h"
#include "BLEValueReceiver.h"
#include "BLEValueWriter.h"
#include "XboxControlsEvent.h"
#include "XboxBatteryEvent.h"
#include "XboxVibrationsCommand.h"

using OnControlsUpdate = std::function<void(XboxControlsEvent& e)>;
using OnBatteryUpdate = std::function<void(XboxBatteryEvent& e)>;

template class BLEValueWriter<XboxVibrationsCommand>;

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
  BLEValueReceiver<XboxControlsEvent> _controls;
  BLEValueReceiver<XboxBatteryEvent> _battery;
  BLEValueWriter<XboxVibrationsCommand> _vibrations;
};
