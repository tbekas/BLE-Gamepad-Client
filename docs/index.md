# BLE-Gamepad-Client

This library enables connecting BLE (Bluetooth Low Energy) gamepads to ESP32 boards. Supported gamepads include the Xbox
Wireless Controller and the Steam Controller.

## Arduino Library

* Open Arduino Library Manager: Tools -> Manage Libraries.
* Search for `BLE-Gamepad-Client` and install it.

## PlatformIO dependency

Add the following line to
the [lib_deps](https://docs.platformio.org/en/latest/projectconf/sections/env/options/library/lib_deps.html) option
of [platformio.ini](https://docs.platformio.org/en/latest/projectconf/index.html) file.

```yaml
tbekas/BLE-Gamepad-Client@^0.11.0
```

## Example usage

### Reading controls in loop

```cpp
#include <Arduino.h>
#include <BLEGamepadClient.h>

XboxController controller;

void setup(void) {
  Serial.begin(115200);
  controller.begin();
}

void loop() {
  if (controller.isConnected()) {
    XboxControlsState s;
    controller.read(&s);

    Serial.printf("lx: %.2f, ly: %.2f, rx: %.2f, ry: %.2f\n",
      s.leftStickX, s.leftStickY, s.rightStickX, s.rightStickY);
  } else {
    Serial.println("controller not connected");
  }
  delay(100);
}
```

### Reading controls using callback

```cpp
#include <Arduino.h>
#include <BLEGamepadClient.h>

XboxController controller;

void onValueChanged(XboxControlsState &s) {
  Serial.printf("lx: %.2f, ly: %.2f, rx: %.2f, ry: %.2f\n",
    s.leftStickX, s.leftStickY, s.rightStickX, s.rightStickY);
}

void setup(void) {
  Serial.begin(115200);
  controller.begin();
  controller.onValueChanged(onValueChanged);
}

void loop() {
  delay(100);
}

```

### More examples

Checkout the code examples in
the [examples directory](https://github.com/tbekas/BLE-Gamepad-Client/tree/0.11.0/examples).
