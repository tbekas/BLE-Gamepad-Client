#pragma once

#include "utils.h"

#include "BLEIncomingSignal.h"
#include "BLEOutgoingSignal.h"

namespace blegc::xbox {

extern const std::string advertisedDeviceName;

extern const CharacteristicFilter controlsCharacteristic;
extern const CharacteristicFilter batteryCharacteristic;
extern const CharacteristicFilter vibrationsCharacteristic;

extern const BLEIncomingSignal<BLEControlsEvent>::Decoder controlsDecoder;
extern const BLEIncomingSignal<BLEBatteryEvent>::Decoder batteryDecoder;
extern const BLEOutgoingSignal<BLEVibrationsCommand>::Encoder vibrationsEncoder;

extern const size_t vibrationsBufferLen;

}  // namespace blegc::xbox
