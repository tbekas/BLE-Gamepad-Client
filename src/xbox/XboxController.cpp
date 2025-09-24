#include "XboxController.h"

#include <NimBLEDevice.h>
#include <bitset>
#include "BLEValueReceiver.h"
#include "utils.h"

using namespace blegc;

XboxController::XboxController(const NimBLEAddress allowedAddress) : BLEBaseController(allowedAddress) {}

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
  if (!discoverAttributes(pClient)) {
    return false;
  }

  auto* pControlsChar = findNotifiableCharacteristic(pClient, hidSvcUUID, inputReportChrUUID);
  auto* pBatteryChar = findNotifiableCharacteristic(pClient, batterySvcUUID, batteryLevelCharUUID);
  auto* pVibrationsChar = findWritableCharacteristic(pClient, hidSvcUUID, inputReportChrUUID);

  return BLEValueReceiver<XboxControlsEvent>::init(pControlsChar) &&
         BLEValueReceiver<XboxBatteryEvent>::init(pBatteryChar) && BLEValueWriter::init(pVibrationsChar);
}
