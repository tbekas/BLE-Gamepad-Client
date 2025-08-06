# Adding support for a controller

To add support for a controller model that is not yet supported, you need to provide an instance of the
`BLEControllerAdapter` struct and register it using `BLEControllerRegistry::addControllerAdapter()`.

Below is a simplified example of an adapter. The `myDecodeControls` function reads the first two bytes from the
`payload` array, scales them from the original range of `0–255` to a normalized range of `-1.0f` to `1.0f`, and assigns
the resulting values to the corresponding `BLEControlsEvent` members representing left stick deflection.

The `serviceUUID` is set to `0x1812`, which is the UUID assigned to the HID (Human Interface Device) service in the
[Bluetooth specification](https://bitbucket.org/bluetooth-SIG/public/src/main/assigned_numbers/uuids/service_uuids.yaml).

```cpp
#include <Arduino.h>
#include <BLEController.h>

BLEController controller;

size_t myDecodeControls(BLEControlsEvent& e, uint8_t payload[], size_t payloadLen) {
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
  myAdapter.controls.decoder = myDecodeControls;

  BLEControllerRegistry::addControllerAdapter(myAdapter);

  controller.begin();
}

void loop() {
  BLEControlsEvent e;

  if (controller.isConnected()) {
    controller.readControls(e);
    Serial.printf("lx: %.2f, ly: %.2f\n", e.leftStickX, e.leftStickY);
  } else {
    Serial.println("controller not connected");
  }

  delay(100);
}
```

You can find a more advanced example in the repository:
[xbox.cpp](https://github.com/tbekas/BLE-Gamepad-Client/tree/0.3.3/src/xbox.cpp)
