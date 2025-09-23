#pragma once

#include <NimBLEDevice.h>
#include "BLEValueReceiver.h"

template <typename T>
class BLEControlsAPI {
 public:
  /**
   * @brief Sets the callback to be invoked when the controller sends update to the controls state.
   * @param callback Reference to the callback function.
   */
  void onControlsUpdate(const std::function<void(T& e)>& callback) { _controls.onUpdate(callback); }

  /**
   * @brief Read the controls state from the connected controller.
   * @param[out] event Reference to the event instance where the data will be written.
   */
  void readControls(T& event) { _controls.readLast(event); }

 protected:
  BLEControlsAPI(const BLEValueDecoder<T>& decoder, const BLECharacteristicSpec& charSpec)
      : _controls(decoder, charSpec) {}

  bool init(NimBLEClient* pClient) { return _controls.init(pClient); }

  BLEValueReceiver<T> _controls;
};
