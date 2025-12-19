#include "XboxController.h"

#include <NimBLEDevice.h>
#include <bitset>
#include "BLEValueReceiver.h"
#include "logger.h"
#include "utils.h"

using namespace blegc;

XboxController::XboxController() = default;
XboxController::~XboxController() = default;

bool XboxController::isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) {
  if (pAdvertisedDevice->haveName()) {
    if (pAdvertisedDevice->getName() == "Xbox Wireless Controller") {
      return true;
    }

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

bool XboxController::init() {
  auto* pControlsChar = findNotifiableCharacteristic(_pClient, hidSvcUUID, inputReportChrUUID);
  auto* pBatteryChar = findNotifiableCharacteristic(_pClient, batterySvcUUID, batteryLevelCharUUID);
  auto* pVibrationsChar = findWritableCharacteristic(_pClient, hidSvcUUID, inputReportChrUUID);

  return BLEValueReceiver<XboxControlsState>::init(pControlsChar) &&
         BLEValueReceiver<XboxBatteryState>::init(pBatteryChar) && BLEValueWriter::init(pVibrationsChar);
}
