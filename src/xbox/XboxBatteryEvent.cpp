#include "XboxBatteryEvent.h"

#include <NimBLEDevice.h>
#include "../BLECharacteristicSpec.h"
#include "../coders.h"
#include "../logger.h"

static auto* LOG_TAG = "XboxControlsEvent";

constexpr size_t batteryDataLen = 1;

BLEDecodeResult decodeBatteryEvent(XboxBatteryEvent& e, uint8_t data[], size_t dataLen) {
  if (dataLen != batteryDataLen) {
    BLEGC_LOGE(LOG_TAG, "Expected %d bytes, was %d bytes", batteryDataLen, dataLen);
    return BLEDecodeResult::InvalidReport;
  }

  e.level = 0.01f * static_cast<float>(data[0]);
  return BLEDecodeResult::Success;
}

const BLEValueDecoder<XboxBatteryEvent> XboxBatteryEvent::Decoder(decodeBatteryEvent);
const BLECharacteristicSpec XboxBatteryEvent::CharSpec{.serviceUUID = NimBLEUUID(blegc::batterySvcUUID),
                                                       .characteristicUUID = NimBLEUUID(blegc::batteryLevelCharUUID),
                                                       .properties = BLE_GATT_CHR_PROP_NOTIFY};
