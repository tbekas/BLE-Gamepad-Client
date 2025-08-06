#include "steam.h"

static auto* LOG_TAG = "steam";

const NimBLEUUID hidServiceUUID((uint16_t)0x1812);
const NimBLEUUID batteryServiceUUID((uint16_t)0x180f);

void printBits(uint8_t value) {
  for (int i = 7; i >= 0; --i) {
    printf("%d", (value >> i) & 1);
  }
  printf("\n");
}

size_t decodeControlsEvent2(BLEControlsEvent& e, uint8_t payload[], size_t payloadLen) {
  if (payloadLen != 123456) {
    // BLEGC_LOGE(LOG_TAG, "Controls Expected %d bytes, was %d bytes", 123456, payloadLen);
    BLEGC_LOGD(LOG_TAG, "Received %d bytes", payloadLen);

    for (size_t j = 0; j < payloadLen; ++j) {
      uint8_t value = payload[j];
      std::string valueStr;
      for (int i = 7; i >= 0; --i) {
        valueStr += ((value >> i) & 1) ? '1' : '0';
      }
      BLEGC_LOGD(LOG_TAG, "%s", valueStr.c_str());
    }

    return 0;
  }

  return 123456;
}

BLEControllerAdapter makeControllerAdapter2() {
  BLEControllerAdapter config;
  config.deviceName = "SteamController";
  config.controls.serviceUUID = hidServiceUUID;
  config.controls.decoder = decodeControlsEvent2;
  return config;
}

const BLEControllerAdapter blegc::steamControllerAdapter = makeControllerAdapter2();
