# Adding support for a controller

To add support for a controller model that is not yet supported, you need to provide an instance of the
`BLEControllerAdapter` struct and register it using `BLEControllerRegistry::addControllerAdapter()`.

Below is a simplified example of an adapter. Its `decoder` reads the first two bytes from the `payload` array, scales
them to the range `-1.0f` to `1.0f`, and assigns them to the corresponding `BLEControlsEvent` members representing left
stick deflection.

```cpp
#include <Arduino.h>
#include <BLEController.h>

BLEController controller;

size_t decodeControls(BLEControlsEvent& e, uint8_t payload[], size_t payloadLen) {
  if (payloadLen < 2) {
    // Not enough data to decode
    return 0;
  }
  e.leftStickX = (2.0f * payload[0]) / 255.0f - 1.0f;
  e.leftStickY = (2.0f * payload[1]) / 255.0f - 1.0f;

  // Return the number of bytes read; any non-zero value indicates success
  return 2;
}

void setup(void) {
  Serial.begin(115200);

  BLEControllerAdapter myAdapter;
  myAdapter.controls.serviceUUID = NimBLEUUID((uint16_t)0x1812);
  myAdapter.controls.decoder = decodeControls;

  BLEControllerRegistry::addControllerAdapter(myAdapter);

  controller.begin();
}

void loop() {
  BLEControlsEvent e;

  if (controller.isConnected()) {
    controller.readControls(e);
    Serial.printf("lx: %.2f, ly: %.2f\n", e.leftStickX, e.leftStickY);
  }

  delay(100);
}
```

You can find a more advanced example in the repository:
[xbox.cpp](https://github.com/tbekas/BLE-Gamepad-Client/tree/0.2.0/src/xbox.cpp)
