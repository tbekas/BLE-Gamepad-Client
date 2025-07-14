#include <NimBLEUUID.h>
#include <string>
#include "ControllerConfig.h"
#include "BatteryEvent.h"
#include "ControlsEvent.h"
#include "Parser.h"
#include "SignalConfig.h"

void ControllerConfig::setControlsConfig(NimBLEUUID serviceUUID, Parser<ControlsEvent> parser) {
  controlsConfig.serviceUUID = serviceUUID;
  controlsConfig.parser = parser;
}

void ControllerConfig::setControlsConfig(NimBLEUUID serviceUUID,
                                               NimBLEUUID characteristicUUID,
                                               Parser<ControlsEvent> parser) {
  controlsConfig.serviceUUID = serviceUUID;
  controlsConfig.characteristicUUID = characteristicUUID;
  controlsConfig.parser = parser;
}

void ControllerConfig::setBatteryConfig(NimBLEUUID serviceUUID, Parser<BatteryEvent> parser) {
  batteryConfig.serviceUUID = serviceUUID;
  batteryConfig.parser = parser;
}

void ControllerConfig::setBatteryConfig(NimBLEUUID serviceUUID,
                                              NimBLEUUID characteristicUUID,
                                              Parser<BatteryEvent> parser) {
  batteryConfig.serviceUUID = serviceUUID;
  batteryConfig.characteristicUUID = characteristicUUID;
  batteryConfig.parser = parser;
}