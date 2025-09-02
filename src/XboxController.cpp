#include "XboxController.h"

#include <NimBLEDevice.h>
#include <bitset>
#include "BLENotifiableSignal.h"
#include "XboxVibrationsCommand.h"
#include "utils.h"

const std::string advertisedDeviceName = "Xbox Wireless Controller";

XboxController::XboxController(const NimBLEAddress allowedAddress)
    : BLEBaseController(allowedAddress),
      _controls(XboxControlsEvent::Decoder, XboxControlsEvent::CharacteristicLocation),
      _battery(XboxBatteryEvent::Decoder, XboxBatteryEvent::CharacteristicLocation),
      _vibrations(XboxVibrationsCommand::Encoder, XboxVibrationsCommand::CharacteristicLocation) {}

XboxController::XboxController() : XboxController(NimBLEAddress()) {}

bool XboxController::isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  return pAdvertisedDevice->haveName() && pAdvertisedDevice->getName() == advertisedDeviceName;
}

bool XboxController::deinit() {
  return true;
}

bool XboxController::init(NimBLEClient* pClient) {
  if (!blegc::discoverAttributes(pClient)) {
    return false;
  }

  return _controls.init(pClient) && _battery.init(pClient) && _vibrations.init(pClient);
}

/**
 * @brief Read the controls state from the connected controller.
 * @param[out] event Reference to the event instance where the data will be written.
 */
void XboxController::readControls(XboxControlsEvent& event) {
  _controls.readLast(event);
}

/**
 * @brief Sets the callback to be invoked when the controller sends update to the controls state.
 * @param callback Reference to the callback function.
 */
void XboxController::onControlsUpdate(const OnControlsUpdate& callback) {
  _controls.onUpdate(callback);
}

/**
 * @brief Read the battery state from the connected controller.
 * @param[out] event Reference to the event instance where the data will be written.
 */
void XboxController::readBattery(XboxBatteryEvent& event) {
  _battery.readLast(event);
}

/**
 * @brief Sets the callback to be invoked when the controller sends update to the battery state.
 * @param callback Reference to the callback function.
 */
void XboxController::onBatteryUpdate(const OnBatteryUpdate& callback) {
  _battery.onUpdate(callback);
}

/**
 * @brief Send the vibrations command to the connected controller.
 * @param cmd Command enabling specific motors in the controller.
 */
void XboxController::writeVibrations(const XboxVibrationsCommand& cmd) {
  _vibrations.write(cmd);
}
