#include "XboxControlsEvent.h"

#include <NimBLEDevice.h>
#include <bitset>
#include "../BLECharacteristicSpec.h"
#include "../logger.h"
#include "../coders.h"

static auto* LOG_TAG = "XboxControlsEvent";

BLEDecodeResult decodeControlsEvent(XboxControlsEvent& e, uint8_t payload[], size_t payloadLen);

const BLEValueDecoder<XboxControlsEvent> XboxControlsEvent::Decoder(decodeControlsEvent);
const BLECharacteristicSpec XboxControlsEvent::CharSpec{
    .serviceUUID = NimBLEUUID(blegc::hidSvcUUID),
    .characteristicUUID = NimBLEUUID(blegc::inputReportChrUUID),
    .properties = BLE_GATT_CHR_PROP_NOTIFY};

constexpr size_t controlsPayloadLen = 16;
constexpr uint16_t axisMax = 0xffff;
constexpr uint16_t triggerMax = 0x3ff;

inline uint16_t make_uint16(uint8_t lsb, uint8_t msb) {
  uint16_t val = msb;
  val <<= 8;
  val += lsb;
  return val;
}

inline float decodeStickX(uint8_t lsb, uint8_t msb) {
  return 2.0f * static_cast<float>(make_uint16(lsb, msb)) / axisMax - 1.0f;
}

inline float decodeStickY(uint8_t lsb, uint8_t msb) {
  return -2.0f * static_cast<float>(make_uint16(lsb, msb)) / axisMax + 1.0f;
}

inline float decodeTrigger(uint8_t lsb, uint8_t msb) {
  return 1.0f * static_cast<float>(make_uint16(lsb, msb)) / triggerMax;
}

inline bool decodeButton(uint8_t byte, int bit) {
  return byte & 1 << bit;
}

BLEDecodeResult decodeControlsEvent(XboxControlsEvent& e, uint8_t payload[], size_t payloadLen) {
  if (payloadLen != controlsPayloadLen) {
    return BLEDecodeResult::InvalidReport;
  }

  e.leftStickX = decodeStickX(payload[0], payload[1]);
  e.leftStickY = decodeStickY(payload[2], payload[3]);
  e.rightStickX = decodeStickX(payload[4], payload[5]);
  e.rightStickY = decodeStickY(payload[6], payload[7]);
  e.leftTrigger = decodeTrigger(payload[8], payload[9]);
  e.rightTrigger = decodeTrigger(payload[10], payload[11]);

  // clang-format off
  e.dpadUp = e.dpadRight = e.dpadDown = e.dpadLeft = false;
  uint8_t byte12 = payload[12];
  switch (byte12) {
    case 1: e.dpadUp = true; break;
    case 2: e.dpadUp = e.dpadRight = true; break;
    case 3: e.dpadRight = true; break;
    case 4: e.dpadRight = e.dpadDown = true; break;
    case 5: e.dpadDown = true; break;
    case 6: e.dpadDown = e.dpadLeft = true; break;
    case 7: e.dpadLeft = true; break;
    case 8: e.dpadLeft = e.dpadUp = true; break;
    default: break;
  }
  // clang-format on

  uint8_t byte13 = payload[13];
  e.buttonA = decodeButton(byte13, 0);
  e.buttonB = decodeButton(byte13, 1);
  e.buttonX = decodeButton(byte13, 3);
  e.buttonY = decodeButton(byte13, 4);
  e.leftBumper = decodeButton(byte13, 6);
  e.rightBumper = decodeButton(byte13, 7);

  uint8_t byte14 = payload[14];
  e.viewButton = decodeButton(byte14, 2);
  e.menuButton = decodeButton(byte14, 3);
  e.xboxButton = decodeButton(byte14, 4);
  e.leftStickButton = decodeButton(byte14, 5);
  e.rightStickButton = decodeButton(byte14, 6);

  uint8_t byte15 = payload[15];
  e.shareButton = decodeButton(byte15, 0);

  return BLEDecodeResult::Success;
}
