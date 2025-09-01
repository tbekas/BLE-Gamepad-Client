#pragma once

#include <NimBLEAddress.h>
#include "BLEBatteryEvent.h"
#include "BLEControlsEvent.h"
#include "BLEIncomingSignal.h"
#include "BLEOutgoingSignal.h"
#include "BLEBaseController.h"

using OnControlsUpdate = std::function<void(BLEControlsEvent& e)>;
using OnBatteryUpdate = std::function<void(BLEBatteryEvent& e)>;

class BLEXboxController : public BLEBaseController {
 public:
  explicit BLEXboxController(NimBLEAddress allowedAddress);
  BLEXboxController();
  ~BLEXboxController() = default;

  // overrides
  bool isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) override;
  bool init(NimBLEClient* pClient) override;
  bool deinit() override;

  void readControls(BLEControlsEvent& event);
  void onControlsUpdate(const OnControlsUpdate& callback);
  void readBattery(BLEBatteryEvent& event);
  void onBatteryUpdate(const OnBatteryUpdate& callback);
  void writeVibrations(const BLEVibrationsCommand& cmd);

 private:
  BLEIncomingSignal<BLEControlsEvent> _controls;
  BLEIncomingSignal<BLEBatteryEvent> _battery;
  BLEOutgoingSignal<BLEVibrationsCommand> _vibrations;
};
