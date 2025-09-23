#pragma once

#include <NimBLEDevice.h>
#include "BLEValueReceiver.h"

template <typename T>
class BLEBatteryAPI {
 public:
  /**
   * @brief Read the battery state from the connected controller.
   * @param[out] event Reference to the event instance where the data will be written.
   */
  void readBattery(T& event) { _battery.readLast(event); }

  /**
   * @brief Sets the callback to be invoked when the controller sends update to the battery state.
   * @param callback Reference to the callback function.
   */
  void onBatteryUpdate(const std::function<void(T& e)>& callback) { _battery.onUpdate(callback); }

 protected:
  BLEBatteryAPI(const BLEValueDecoder<T>& decoder, const BLECharacteristicSpec& charSpec) : _battery(decoder, charSpec) {}

  bool init(NimBLEClient* pClient) { return _battery.init(pClient); }

  BLEValueReceiver<T> _battery;
};
