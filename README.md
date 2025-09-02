# BLE-Gamepad-Client

This library enables connecting BLE (Bluetooth Low Energy) gamepads to ESP32 boards. Currently, only the Xbox Wireless
Controller is supported.

# Arduino Library

* Open Arduino Library Manager: Tools -> Manage Libraries.
* Search for `BLE-Gamepad-Client` and install it.

# PlatformIO dependency

Add the following line to
the [lib_deps](https://docs.platformio.org/en/latest/projectconf/sections/env/options/library/lib_deps.html) option
of [platformio.ini](https://docs.platformio.org/en/latest/projectconf/index.html) file.

```yaml
tbekas/BLE-Gamepad-Client@^0.5.0
```

# Example usage

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
    XboxControlsEvent e;
    controller.readControls(e);

    Serial.printf("lx: %.2f, ly: %.2f, rx: %.2f, ry: %.2f\n",
      e.leftStickX, e.leftStickY, e.rightStickX, e.rightStickY);
  } else {
    Serial.println("controller not connected");
  }
  delay(100);
}
```

# Step-by-step tutorials

### How to Connect an Xbox Controller to ESP32 Using Arduino IDE
[![How to Connect an Xbox Controller to ESP32 Using Arduino IDE](https://img.youtube.com/vi/5oH3JBZrI9c/mqdefault.jpg)](https://www.youtube.com/watch?v=5oH3JBZrI9c)

### How to Connect an Xbox Controller to ESP32 Using VSCode and PlatformIO
[![How to Connect an Xbox Controller to ESP32 Using VSCode and PlatformIO](https://img.youtube.com/vi/eePqTX-07oo/mqdefault.jpg)](https://www.youtube.com/watch?v=eePqTX-07oo)

# More examples

Checkout the code examples in
the [examples directory](https://github.com/tbekas/BLE-Gamepad-Client/tree/0.5.0/examples).

# Supported gamepads

### Xbox One Wireless Controller (model 1708)

Pairing instructions are the same as for model 1914. If controller is not pairing, you probably need to
update the controller's firmware to version 5.x
using [these instructions](https://support.xbox.com/en-US/help/hardware-network/controller/update-xbox-wireless-controller).

### Xbox Series S/X Wireless Controller (model 1914)

Pairing instructions:

* Turn on your controller by pressing the Xbox button.
* Press and hold the controller’s pair button for 3 seconds, then release.

# Acknowledgments

* [h2zero](https://github.com/h2zero) for the excellent [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) library, which
  this library is built upon.
