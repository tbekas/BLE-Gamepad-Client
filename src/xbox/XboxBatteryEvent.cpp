#include "XboxBatteryEvent.h"

#include <NimBLEDevice.h>
#include "../logger.h"
#include "../utils.h"

static auto* LOG_TAG = "XboxControlsEvent";

constexpr size_t batteryPayloadLen = 1;

blegc::BLEDecodeResult decodeBatteryEvent(XboxBatteryEvent& e, uint8_t payload[], size_t payloadLen) {
  if (payloadLen != batteryPayloadLen) {
    BLEGC_LOGE(LOG_TAG, "Expected %d bytes, was %d bytes", batteryPayloadLen, payloadLen);
    return blegc::BLEDecodeResult::InvalidReport;
  }

  e.level = 0.01f * static_cast<float>(payload[0]);
  return blegc::BLEDecodeResult::Success;
}

const blegc::BLEValueDecoder<XboxBatteryEvent> XboxBatteryEvent::Decoder(decodeBatteryEvent);
const blegc::BLECharacteristicSpec XboxBatteryEvent::CharSpec{
    .serviceUUID = NimBLEUUID(blegc::batterySvcUUID),
    .characteristicUUID = NimBLEUUID(blegc::batteryLevelCharUUID),
    .properties = BLE_GATT_CHR_PROP_NOTIFY};
