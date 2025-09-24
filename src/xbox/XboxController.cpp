#include "XboxController.h"

#include <NimBLEDevice.h>
#include <bitset>
#include "BLEValueReceiver.h"
#include "utils.h"
#include "XboxVibrationsCommand.h"

XboxController::XboxController(const NimBLEAddress allowedAddress)
    : BLEBaseController(allowedAddress),
      BLEValueReceiver<XboxControlsEvent>(XboxControlsEvent::Decoder, XboxControlsEvent::CharSpec),
      BLEValueReceiver<XboxBatteryEvent>(XboxBatteryEvent::Decoder, XboxBatteryEvent::CharSpec),
      BLEValueWriter(XboxVibrationsCommand::Encoder, XboxVibrationsCommand::CharSpec) {}

XboxController::XboxController(const std::string& allowedAddress)
    : XboxController(NimBLEAddress(allowedAddress, BLE_ADDR_PUBLIC)) {}

XboxController::XboxController() : XboxController(NimBLEAddress()) {}

bool XboxController::isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  return pAdvertisedDevice->haveName() && pAdvertisedDevice->getName() == "Xbox Wireless Controller";
}

bool XboxController::deinit() {
  return true;
}

bool XboxController::init(NimBLEClient* pClient) {
  if (!blegc::discoverAttributes(pClient)) {
    return false;
  }

  return BLEValueReceiver<XboxControlsEvent>::init(pClient) && BLEValueReceiver<XboxBatteryEvent>::init(pClient) &&
         BLEValueWriter::init(pClient);
}
