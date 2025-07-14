#include <bitset>
#include "BatteryEvent.h"
#include "ControllerConfig.h"
#include "ControlsEvent.h"
#include "Logger.h"

namespace xbox {
const NimBLEUUID deviceInfoServiceUUID((uint16_t)0x180a);
const NimBLEUUID batteryServiceUUID((uint16_t)0x180f);
const NimBLEUUID hidServiceUUID((uint16_t)0x1812);

const uint16_t axisMax = 0xffff;
const uint16_t triggerMax = 0x3ff;

inline float axisX(uint16_t val) {
  return ((2.0f * val) / axisMax) - 1.0f;
}

inline float axisY(uint16_t val) {
  return ((-2.0f * val) / axisMax) + 1.0f;
}

inline float trigger(uint16_t val) {
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

inline bool button(uint8_t byte, int bit) {
  return byte & mask(bit);
}

inline void printBits(uint8_t byte, int label) {
  auto bits = std::bitset<8>(byte);
  BLEGC_LOGI("Byte %d: %s", label, bits.to_string().c_str());
}

ControlsEvent parseControlsData(uint8_t pData[], size_t length) {
  ControlsEvent e;

  if (length != 16) {
    BLEGC_LOGE("Expected 16 bytes, was %d bytes", length);
    return e;
  }

  e.lx = axisX(uint16(pData[0], pData[1]));
  e.ly = axisY(uint16(pData[2], pData[3]));
  e.rx = axisX(uint16(pData[4], pData[5]));
  e.ry = axisY(uint16(pData[6], pData[7]));
  e.lt = trigger(uint16(pData[8], pData[9]));
  e.rt = trigger(uint16(pData[10], pData[11]));

  // clang-format off
  e.dpadUp = e.dpadRight = e.dpadDown = e.dpadLeft = false;
  uint8_t dpad = pData[12];
  switch (dpad) {
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

  uint8_t btns1 = pData[13];
  e.buttonA = button(btns1, 0);
  e.buttonB = button(btns1, 1);
  e.buttonX = button(btns1, 3);
  e.buttonY = button(btns1, 4);
  e.lb = button(btns1, 6);
  e.rb = button(btns1, 7);

  uint8_t btns2 = pData[14];
  e.view = button(btns2, 2);
  e.menu = button(btns2, 3);
  e.xbox = button(btns2, 4);
  e.lsb = button(btns2, 5);
  e.rsb = button(btns2, 6);

  uint8_t btns3 = pData[15];
  e.share = button(btns3, 0);

  return e;
}

BatteryEvent parseBatteryData(uint8_t pData[], size_t length) {
  BatteryEvent e;

  if (length != 1) {
    BLEGC_LOGE("Expected 1 byte, was %d bytes", length);
    return e;
  }

  e.level = (0.01f * pData[0]);

  return e;
}

ControllerConfig controllerConfig() {
  ControllerConfig config;
  config.deviceName = "Xbox Wireless Controller";
  config.setControlsConfig(hidServiceUUID, parseControlsData);
  config.setBatteryConfig(batteryServiceUUID, parseBatteryData);
  return config;
}
}  // namespace xbox
