#include "xbox.h"
#include <bitset>
#include "BLEBatteryEvent.h"
#include "BLEControllerAdapter.h"
#include "BLEControlsEvent.h"
#include "BLEVibrationsCommand.h"
#include "logger.h"

const NimBLEUUID hidServiceUUID((uint16_t)0x1812);
const NimBLEUUID batteryServiceUUID((uint16_t)0x180f);

constexpr size_t controlsPayloadLen = 16;
constexpr size_t batteryPayloadLen = 1;
constexpr size_t vibrationsPayloadLen = 8;

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
  return uint8_t(1) << bit;
}

inline bool decodeButton(uint8_t byte, int bit) {
  return byte & mask(bit);
}

inline void printBits(uint8_t byte, int label) {
  auto bits = std::bitset<8>(byte);
  BLEGC_LOGI("Byte %d: %s", label, bits.to_string().c_str());
}

size_t decodeControlsEvent(BLEControlsEvent& e, uint8_t payload[], size_t payloadLen) {
  if (payloadLen != controlsPayloadLen) {
    BLEGC_LOGE("Expected %d bytes, was %d bytes", controlsPayloadLen, payloadLen);
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

size_t decodeBatteryEvent(BLEBatteryEvent& e, uint8_t payload[], size_t payloadLen) {
  if (payloadLen != batteryPayloadLen) {
    BLEGC_LOGE("Expected %d bytes, was %d bytes", batteryPayloadLen, payloadLen);
    return 0;
  }

  e.level = 0.01f * static_cast<float>(payload[0]);
  return batteryPayloadLen;
}

inline uint8_t encodeMotorEnable(float power, int bit) {
  return (power > 0.0f) * mask(bit);
}

inline uint8_t encodeMotorPower(float power) {
  return static_cast<uint8_t>(max(min(power, 1.0f), 0.0f) * 100.0f);
}

inline uint8_t encodeDuration(uint32_t durationMs) {
  return static_cast<uint8_t>(min(durationMs, uint32_t(2550)) / 10);
}

size_t encodeVibrationsCommand(const BLEVibrationsCommand& c, uint8_t outBuffer[], size_t bufferLen) {
  if (bufferLen < vibrationsPayloadLen) {
    BLEGC_LOGW("Expected buffer of at least %d bytes, was %d bytes", vibrationsPayloadLen, bufferLen);
    return 0;
  }

  if (c.cycles == 0) {
    outBuffer[0] = 0;
  } else {
    outBuffer[0] = encodeMotorEnable(c.rightMotor, 0) | encodeMotorEnable(c.leftMotor, 1) |
                   encodeMotorEnable(c.rightTriggerMotor, 2) | encodeMotorEnable(c.leftTriggerMotor, 3);
  }

  outBuffer[1] = encodeMotorPower(c.leftTriggerMotor);
  outBuffer[2] = encodeMotorPower(c.rightTriggerMotor);
  outBuffer[3] = encodeMotorPower(c.leftMotor);
  outBuffer[4] = encodeMotorPower(c.rightMotor);
  outBuffer[5] = encodeDuration(c.durationMs);
  outBuffer[6] = encodeDuration(c.pauseMs);
  outBuffer[7] = c.cycles == 0 ? 0 : c.cycles - 1;

  return vibrationsPayloadLen;
}

BLEControllerAdapter makeControllerAdapter() {
  BLEControllerAdapter config;
  config.deviceName = "Xbox Wireless Controller";
  config.controls.serviceUUID = hidServiceUUID;
  config.controls.decoder = decodeControlsEvent;
  config.battery.serviceUUID = batteryServiceUUID;
  config.battery.decoder = decodeBatteryEvent;
  config.vibrations.serviceUUID = hidServiceUUID;
  config.vibrations.encoder = encodeVibrationsCommand;
  config.vibrations.bufferLen = vibrationsPayloadLen;
  return config;
}

const BLEControllerAdapter blegc::xboxControllerAdapter = makeControllerAdapter();
