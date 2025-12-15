#include "XboxControlsState.h"

#include <NimBLEDevice.h>
#include <bitset>
#include "logger.h"

constexpr size_t controlsDataLen = 16;
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

BLEDecodeResult XboxControlsState::decode(uint8_t data[], size_t dataLen) {
  if (dataLen != controlsDataLen) {
    return BLEDecodeResult::InvalidReport;
  }

  this->leftStickX = decodeStickX(data[0], data[1]);
  this->leftStickY = decodeStickY(data[2], data[3]);
  this->rightStickX = decodeStickX(data[4], data[5]);
  this->rightStickY = decodeStickY(data[6], data[7]);
  this->leftTrigger = decodeTrigger(data[8], data[9]);
  this->rightTrigger = decodeTrigger(data[10], data[11]);

  // clang-format off
  this->dpadUp = this->dpadRight = this->dpadDown = this->dpadLeft = false;
  uint8_t byte12 = data[12];
  switch (byte12) {
    case 1: this->dpadUp = true; break;
    case 2: this->dpadUp = this->dpadRight = true; break;
    case 3: this->dpadRight = true; break;
    case 4: this->dpadRight = this->dpadDown = true; break;
    case 5: this->dpadDown = true; break;
    case 6: this->dpadDown = this->dpadLeft = true; break;
    case 7: this->dpadLeft = true; break;
    case 8: this->dpadLeft = this->dpadUp = true; break;
    default: break;
  }
  // clang-format on

  uint8_t byte13 = data[13];
  this->buttonA = decodeButton(byte13, 0);
  this->buttonB = decodeButton(byte13, 1);
  this->buttonX = decodeButton(byte13, 3);
  this->buttonY = decodeButton(byte13, 4);
  this->leftBumper = decodeButton(byte13, 6);
  this->rightBumper = decodeButton(byte13, 7);

  uint8_t byte14 = data[14];
  this->viewButton = decodeButton(byte14, 2);
  this->menuButton = decodeButton(byte14, 3);
  this->xboxButton = decodeButton(byte14, 4);
  this->leftStickButton = decodeButton(byte14, 5);
  this->rightStickButton = decodeButton(byte14, 6);

  uint8_t byte15 = data[15];
  this->shareButton = decodeButton(byte15, 0);

  return BLEDecodeResult::Success;
}

bool XboxControlsState::operator==(const XboxControlsState& rhs) const {
  // clang-format off
  return
    this->controllerAddress == rhs.controllerAddress &&
    this->leftStickX == rhs.leftStickX &&
    this->leftStickY == rhs.leftStickY &&
    this->rightStickX == rhs.rightStickX &&
    this->rightStickY == rhs.rightStickY &&
    this->leftStickButton == rhs.leftStickButton &&
    this->rightStickButton == rhs.rightStickButton &&
    this->dpadUp == rhs.dpadUp &&
    this->dpadDown == rhs.dpadDown &&
    this->dpadLeft == rhs.dpadLeft &&
    this->dpadRight == rhs.dpadRight &&
    this->buttonA == rhs.buttonA &&
    this->buttonB == rhs.buttonB &&
    this->buttonX == rhs.buttonX &&
    this->buttonY == rhs.buttonY &&
    this->leftBumper == rhs.leftBumper &&
    this->rightBumper == rhs.rightBumper &&
    this->leftTrigger == rhs.leftTrigger &&
    this->rightTrigger == rhs.rightTrigger &&
    this->shareButton == rhs.shareButton &&
    this->menuButton == rhs.menuButton &&
    this->viewButton == rhs.viewButton &&
    this->xboxButton == rhs.xboxButton;
  // clang-format on
}
bool XboxControlsState::operator!=(const XboxControlsState& rhs) const {
  return !(*this == rhs);
}
