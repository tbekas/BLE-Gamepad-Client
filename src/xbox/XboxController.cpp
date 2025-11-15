#include "XboxController.h"

#include <NimBLEDevice.h>
#include <bitset>
#include "BLEValueReceiver.h"
#include "logger.h"
#include "utils.h"

using namespace blegc;

XboxController::XboxController(const NimBLEAddress allowedAddress) : BLEBaseController(allowedAddress) {}

XboxController::XboxController(const std::string& allowedAddress)
    : XboxController(NimBLEAddress(allowedAddress, BLE_ADDR_PUBLIC)) {}

XboxController::XboxController() : XboxController(NimBLEAddress()) {}

bool XboxController::isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  if (!pAdvertisedDevice->haveServiceUUID()) {
    BLEGC_LOGD("Service uuids missing");
    return false;
  }

  if (!pAdvertisedDevice->isAdvertisingService(hidSvcUUID)) {
    BLEGC_LOGD("HID service missing, uuid: 0x%02x", hidSvcUUID);
    return false;
  }

  // if the name is present it has to be `Xbox Wireless Controller`
  if (pAdvertisedDevice->haveName() && pAdvertisedDevice->getName() != "Xbox Wireless Controller") {
    BLEGC_LOGD("Name mismatch: %s", pAdvertisedDevice->getName().c_str());
    return false;
  }

  if (!pAdvertisedDevice->haveAppearance()) {
    BLEGC_LOGD("Appearance missing");
    return false;
  }

  if (pAdvertisedDevice->getAppearance() != gamepadAppearance) {
    BLEGC_LOGD("Appearance mismatch: 0x%02x", pAdvertisedDevice->getAppearance());
    return false;
  }

  if (!pAdvertisedDevice->haveManufacturerData()) {
    BLEGC_LOGD("Manufacturer id missing");
    return false;
  }

  if (getManufacturerId(pAdvertisedDevice) != microsoftCompanyId) {
    BLEGC_LOGD("Manufacturer id mismatch: 0x%02x", getManufacturerId(pAdvertisedDevice));
    return false;
  }

  return true;
}

bool XboxController::deinit() {
  return true;
}

bool XboxController::init(NimBLEClient* pClient) {
  auto* pControlsChar = findNotifiableCharacteristic(pClient, hidSvcUUID, inputReportChrUUID);
  auto* pBatteryChar = findNotifiableCharacteristic(pClient, batterySvcUUID, batteryLevelCharUUID);
  auto* pVibrationsChar = findWritableCharacteristic(pClient, hidSvcUUID, inputReportChrUUID);

  return BLEValueReceiver<XboxControlsEvent>::init(pControlsChar) &&
         BLEValueReceiver<XboxBatteryEvent>::init(pBatteryChar) && BLEValueWriter::init(pVibrationsChar);
}
