#include <bitset>
#include "BatteryEvent.h"
#include "ControllerConfig.h"
#include "ControlsEvent.h"
#include "VibrationsCommand.h"
#include "Logger.h"

namespace xbox {

const NimBLEUUID hidServiceUUID((uint16_t)0x1812);
const NimBLEUUID batteryServiceUUID((uint16_t)0x180f);

constexpr size_t controlsPayloadLen = 16;
constexpr size_t batteryPayloadLen = 1;
constexpr size_t vibrationsPayloadLen = 8;

constexpr uint16_t axisMax = 0xffff;
constexpr uint16_t triggerMax = 0x3ff;

inline float decodeAxisX(uint16_t val) {
  return ((2.0f * val) / axisMax) - 1.0f;
}

inline float decodeAxisY(uint16_t val) {
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

bool decodeControlsData(ControlsEvent& e, uint8_t pData[], size_t length) {
  if (length != controlsPayloadLen) {
    BLEGC_LOGE("Expected %d bytes, was %d bytes", controlsPayloadLen, length);
    return false;
  }

  e.lx = decodeAxisX(uint16(pData[0], pData[1]));
  e.ly = decodeAxisY(uint16(pData[2], pData[3]));
  e.rx = decodeAxisX(uint16(pData[4], pData[5]));
  e.ry = decodeAxisY(uint16(pData[6], pData[7]));
  e.lt = decodeTrigger(uint16(pData[8], pData[9]));
  e.rt = decodeTrigger(uint16(pData[10], pData[11]));

  // clang-format off
  e.dpadUp = e.dpadRight = e.dpadDown = e.dpadLeft = false;
  uint8_t byte12 = pData[12];
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

  uint8_t byte13 = pData[13];
  e.buttonA = decodeButton(byte13, 0);
  e.buttonB = decodeButton(byte13, 1);
  e.buttonX = decodeButton(byte13, 3);
  e.buttonY = decodeButton(byte13, 4);
  e.lb = decodeButton(byte13, 6);
  e.rb = decodeButton(byte13, 7);

  uint8_t byte14 = pData[14];
  e.view = decodeButton(byte14, 2);
  e.menu = decodeButton(byte14, 3);
  e.xbox = decodeButton(byte14, 4);
  e.lsb = decodeButton(byte14, 5);
  e.rsb = decodeButton(byte14, 6);

  uint8_t byte15 = pData[15];
  e.share = decodeButton(byte15, 0);

  return true;
}

bool decodeBatteryData(BatteryEvent& e, uint8_t pData[], size_t length) {
  if (length != batteryPayloadLen) {
    BLEGC_LOGE("Expected %d bytes, was %d bytes", batteryPayloadLen, length);
    return false;
  }

  e.level = 0.01f * static_cast<float>(pData[0]);
  return true;
}

inline uint8_t encodeSelect(bool val, int bit) {
  return val * mask(bit);
}

inline uint8_t encodePower(float power) {
  return static_cast<uint8_t>(power * 100.0f);
}

size_t encodeVibrationsCommand(const VibrationsCommand& c, uint8_t outBuffer[], size_t bufferLen) {
  if (bufferLen < vibrationsPayloadLen) {
    BLEGC_LOGW("Expected buffer of at least %d bytes, was %d bytes", vibrationsPayloadLen, bufferLen);
    return 0;
  }

  outBuffer[0] =
      encodeSelect(c.selectCenter, 0) |
        encodeSelect(c.selectShake, 1) |
          encodeSelect(c.selectRight, 2) |
            encodeSelect(c.selectLeft, 3);

  outBuffer[1] = encodePower(c.powerLeft);
  outBuffer[2] = encodePower(c.powerRight);
  outBuffer[3] = encodePower(c.powerShake);
  outBuffer[4] = encodePower(c.powerCenter);
  outBuffer[5] = c.timeActiveMs;
  outBuffer[6] = c.timeInactiveMs;
  outBuffer[7] = c.repeat;

  return vibrationsPayloadLen;
}

ControllerConfig controllerConfig() {
  ControllerConfig config;
  config.deviceName = "Xbox Wireless Controller";
  config.controls.serviceUUID = hidServiceUUID;
  config.controls.decoder = decodeControlsData;
  config.battery.serviceUUID = batteryServiceUUID;
  config.battery.decoder = decodeBatteryData;
  config.vibrations.serviceUUID = hidServiceUUID;
  config.vibrations.encoder = encodeVibrationsCommand;
  config.vibrations.bufferLen = vibrationsPayloadLen;

  // config.vibrations.characteristicUUID = NimBLEUUID(static_cast<uint16_t>(0x2a4d));
  return config;
}
}  // namespace xbox
