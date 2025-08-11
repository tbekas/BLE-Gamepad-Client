#include "steam.h"
#include "BLEBatteryEvent.h"
#include "BLEControllerModel.h"
#include "BLEControlsEvent.h"
#include "BLEVibrationsCommand.h"
#include "logger.h"

static auto* LOG_TAG = "steam";

const NimBLEUUID hidServiceUUID((uint16_t)0x1812);
const NimBLEUUID batteryServiceUUID((uint16_t)0x180f);

void printBits(uint8_t value) {
    for (int i = 7; i >= 0; --i) {
        printf("%d", (value >> i) & 1);
    }
    printf("\n");
}

size_t decodeControlsEvent(const int i, BLEControlsEvent& e, uint8_t payload[], size_t payloadLen) {
    if (payloadLen != 123456) {
        // BLEGC_LOGE(LOG_TAG, "Controls Expected %d bytes, was %d bytes", 123456, payloadLen);
        BLEGC_LOGD(LOG_TAG, "%d Received %d bytes", i, payloadLen);

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

size_t decodeControlsEvent1(BLEControlsEvent& e, uint8_t payload[], size_t payloadLen) {
  return decodeControlsEvent(1, e, payload, payloadLen);
}

size_t decodeControlsEvent2(BLEControlsEvent& e, uint8_t payload[], size_t payloadLen) {
  return decodeControlsEvent(2, e, payload, payloadLen);
}

size_t decodeControlsEvent3(BLEControlsEvent& e, uint8_t payload[], size_t payloadLen) {
  return decodeControlsEvent(3, e, payload, payloadLen);
}



BLEControllerModel makeSteamControllerModel() {
    BLEControllerModel m;
    m.advertisedName = "SteamController";
    m.controls.emplace_back(hidServiceUUID, NimBLEUUID(), decodeControlsEvent1);
    m.controls.emplace_back(hidServiceUUID, NimBLEUUID(), decodeControlsEvent2);
    m.controls.emplace_back(hidServiceUUID, NimBLEUUID(), decodeControlsEvent3);
    return m;
}

const BLEControllerModel blegc::steamControllerModel = makeSteamControllerModel();
