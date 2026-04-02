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
tbekas/BLE-Gamepad-Client@^0.12.1
```

## Supported gamepads

### Xbox Wireless Controller

![Xbox One Controller](xbox_one_controller.png)
![Xbox Series Controller](xbox_series_controller.png)

Both Xbox One Wireless Controllers and Xbox Series S/X Wireless Controllers are supported.

#### Firmware

Update (or verify) controller firmware to version 5.x using these
instructions: [Update your Xbox Wireless Controller](https://support.xbox.com/en-US/help/hardware-network/controller/update-xbox-wireless-controller).

#### Example usage

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

    Serial.printf("lstick: %.2f,%.2f, rstick: %.2f,%.2f\n",
      s.leftStickX, s.leftStickY, s.rightStickX, s.rightStickY);
  } else {
    Serial.println("controller not connected");
  }
  delay(100);
}
```

#### Pairing instructions

* Turn on your controller by pressing the Xbox button.
* Press and hold the controller’s pair button for 3 seconds, then release.

### Steam Controller

![Steam Controller](steam_controller.png)

#### Firmware

Install BLE firmware using these instructions: [Steam Controller BLE](https://help.steampowered.com/en/faqs/view/1796-5FC3-88B3-C85F).

#### Example usage

```cpp
#include <Arduino.h>
#include <BLEGamepadClient.h>

SteamController controller;

void setup(void) {
  Serial.begin(115200);
  controller.begin();
}

void loop() {
  if (controller.isConnected()) {
    SteamControlsState s;
    controller.read(&s);

    Serial.printf("stick: %.2f,%.2f, lpad: %.2f,%.2f, rpad: %.2f,%.2f\n",
      s.stickX, s.stickY, s.leftPadX, s.leftPadY, s.rightPadX, s.rightPadY);
  } else {
    Serial.println("controller not connected");
  }
  delay(100);
}
```

#### Pairing instructions

* Turn on your controller by pressing the Steam button while holding the Y button.
