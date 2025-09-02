#include "XboxControlsEvent.h"

#include <NimBLEDevice.h>
#include <bitset>
#include "../logger.h"
#include "../utils.h"

static auto* LOG_TAG = "XboxControlsEvent";

size_t decodeControlsEvent(XboxControlsEvent& e, uint8_t payload[], size_t payloadLen);

const blegc::BLEValueDecoder<XboxControlsEvent> XboxControlsEvent::Decoder(decodeControlsEvent);
const blegc::BLECharacteristicLocation XboxControlsEvent::CharacteristicLocation{
    .serviceUUID = NimBLEUUID(uint16_t{0x1812}),
    .characteristicUUID = NimBLEUUID(uint16_t{0x2a4d}),
    .properties = uint8_t{BLE_GATT_CHR_PROP_NOTIFY}};

constexpr size_t controlsPayloadLen = 16;
constexpr uint16_t axisMax = 0xffff;
constexpr uint16_t triggerMax = 0x3ff;

inline float decodeStickX(uint16_t val) {
  return ((2.0f * val) / axisMax) - 1.0f;
}

inline float decodeStickY(uint16_t val) {
  return ((-2.0f * val) / axisMax) + 1.0f;
}

inline float decodeTrigger(uint16_t val) {
  return (1.0f * val) / triggerMax;
}

inline uint16_t uint16(uint8_t r, uint8_t l) {
  uint16_t val = l;
  val <<= 8;
  val += r;
  return val;
}

constexpr uint8_t mask(int bit) {
  return static_cast<uint8_t>(1) << bit;
}

inline bool decodeButton(uint8_t byte, int bit) {
  return byte & mask(bit);
}

size_t decodeControlsEvent(XboxControlsEvent& e, uint8_t payload[], size_t payloadLen) {
  if (payloadLen != controlsPayloadLen) {
    BLEGC_LOGE(LOG_TAG, "Expected %d bytes, was %d bytes", controlsPayloadLen, payloadLen);
    return 0;
  }

  e.leftStickX = decodeStickX(uint16(payload[0], payload[1]));
  e.leftStickY = decodeStickY(uint16(payload[2], payload[3]));
  e.rightStickX = decodeStickX(uint16(payload[4], payload[5]));
  e.rightStickY = decodeStickY(uint16(payload[6], payload[7]));
  e.leftTrigger = decodeTrigger(uint16(payload[8], payload[9]));
  e.rightTrigger = decodeTrigger(uint16(payload[10], payload[11]));

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
  e.view = decodeButton(byte14, 2);
  e.menu = decodeButton(byte14, 3);
  e.xbox = decodeButton(byte14, 4);
  e.leftStickButton = decodeButton(byte14, 5);
  e.rightStickButton = decodeButton(byte14, 6);

  uint8_t byte15 = payload[15];
  e.share = decodeButton(byte15, 0);

  return controlsPayloadLen;
}
