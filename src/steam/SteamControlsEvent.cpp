#include "SteamControlsEvent.h"

#include <NimBLEDevice.h>
#include <bitset>
#include "logger.h"

// Sources
// https://github.com/torvalds/linux/blob/master/drivers/hid/hid-steam.c
// https://github.com/ricardoquesada/bluepad32/blob/main/src/components/bluepad32/parser/uni_hid_parser_steam.c

#define REPORT_TYPE_INPUT 0x04
#define CONTAINS_BUTTONS_DATA 0x0010
#define CONTAINS_TRIGGERS_DATA 0x0020
#define CONTAINS_THUMBSTICK_DATA 0x0080
#define CONTAINS_LEFT_PAD_DATA 0x0100
#define CONTAINS_RIGHT_PAD_DATA 0x0200
#define CONTAINS_GYRO_DATA 0x1000

constexpr size_t controlsDataLen = 19;

inline uint16_t make_uint16(const uint8_t lsb, const uint8_t msb) {
  uint16_t val = msb;
  val <<= 8;
  val += lsb;
  return val;
}

inline int16_t make_int16(const uint8_t lsb, const uint8_t msb) {
  int16_t val = msb;
  val <<= 8;
  val += lsb;
  return val;
}

inline bool decodeButton(uint8_t byte, int bit) {
  return byte & 1 << bit;
}

inline float decodePad(const uint8_t lsb, const uint8_t msb) {
  return static_cast<float>(make_int16(lsb, msb)) / INT16_MAX;
}

inline float decodeTrigger(const uint8_t b) {
  return static_cast<float>(b) / UINT8_MAX;
}

BLEDecodeResult SteamControlsEvent::decode(uint8_t data[], size_t dataLen) {
  if (dataLen != controlsDataLen) {
    return BLEDecodeResult::InvalidReport;
  }

  if (data[0] != 0xc0) {
    return BLEDecodeResult::NotSupported;
  }

  if (data[1] & 0x0f != REPORT_TYPE_INPUT) {
    return BLEDecodeResult::NotSupported;
  }

  const auto contentInfo = make_uint16(data[1], data[2]);

  auto offset = 3;
  if (contentInfo & CONTAINS_BUTTONS_DATA) {
    const uint8_t byte0 = data[offset];
    this->rightTriggerButton = decodeButton(byte0, 0);
    this->leftTriggerButton = decodeButton(byte0, 1);
    this->rightBumper = decodeButton(byte0, 2);
    this->leftBumper = decodeButton(byte0, 3);
    this->buttonY = decodeButton(byte0, 4);
    this->buttonB = decodeButton(byte0, 5);
    this->buttonX = decodeButton(byte0, 6);
    this->buttonA = decodeButton(byte0, 7);

    const uint8_t byte1 = data[offset + 1];

    this->dpadUp = decodeButton(byte1, 0);
    this->dpadRight = decodeButton(byte1, 1);
    this->dpadLeft = decodeButton(byte1, 2);
    this->dpadDown = decodeButton(byte1, 3);
    this->selectButton = decodeButton(byte1, 4);
    this->steamButton = decodeButton(byte1, 5);
    this->startButton = decodeButton(byte1, 6);
    this->leftGripButton = decodeButton(byte1, 7);

    const uint8_t byte2 = data[offset + 2];
    this->rightGripButton = decodeButton(byte2, 0);
    this->leftPadClick = decodeButton(byte2, 1);
    this->rightPadClick = decodeButton(byte2, 2);
    this->leftPadTouch = decodeButton(byte2, 3);
    this->rightPadTouch = decodeButton(byte2, 4);
    this->stickButton = decodeButton(byte2, 6);
    offset += 3;
  }

  if (contentInfo & CONTAINS_TRIGGERS_DATA) {
    this->leftTrigger = decodeTrigger(data[offset]);
    this->rightTrigger = decodeTrigger(data[offset + 1]);
    offset += 2;
  }

  if (contentInfo & CONTAINS_THUMBSTICK_DATA) {
    this->stickX = decodePad(data[offset], data[offset + 1]);
    this->stickY = decodePad(data[offset + 2], data[offset + 3]);
    offset += 4;
  }

  if (contentInfo & CONTAINS_LEFT_PAD_DATA) {
    this->leftPadX = decodePad(data[offset], data[offset + 1]);
    this->leftPadY = decodePad(data[offset + 2], data[offset + 3]);
    offset += 4;
  }

  if (contentInfo & CONTAINS_RIGHT_PAD_DATA) {
    this->rightPadX = decodePad(data[offset], data[offset + 1]);
    this->rightPadY = decodePad(data[offset + 2], data[offset + 3]);
    offset += 4;
  }

  return BLEDecodeResult::Success;
}
