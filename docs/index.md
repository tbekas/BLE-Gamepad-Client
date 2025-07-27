# Overview
This is a C++ library for Espressif ESP32 boards that enables connecting BLE (Bluetooth Low Energy) gamepads to the board.
It is built on top of the [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) library.

# PlatformIO dependency
Add the following line to the [lib_deps](https://docs.platformio.org/en/latest/projectconf/sections/env/options/library/lib_deps.html) option of [platformio.ini](https://docs.platformio.org/en/latest/projectconf/index.html) file.
```yaml
tbekas/BLE-Gamepad-Client@^0.1.0
```

# Example usage
```cpp
#include <Arduino.h>
#include <BLEGamepadClient.h>

Controller controller;

void setup(void) {
  Serial.begin(115200);
  controller.begin();
}

void loop() {
  ControlsEvent e;

  if (controller.isConnected()) {
    controller.readControls(e);
    Serial.printf("lx: %.2f, ly: %.2f\n", e.leftStickX, e.leftStickY);
  }

  delay(100);
}
```

# More examples
Checkout the code examples in the [examples directory](https://github.com/tbekas/BLE-Gamepad-Client/tree/main/examples).
