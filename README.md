# BLE-Gamepad-Client
Enables connection of BLE gamepads to ESP32 boards. Currently, only the Xbox Wireless Controller is supported.

# PlatformIO dependency
* Add the following line to the [lib_deps](https://docs.platformio.org/en/latest/projectconf/sections/env/options/library/lib_deps.html) option of `[env:]` section of [platformio.ini](https://docs.platformio.org/en/latest/projectconf/index.html) file.
```yaml
tbekas/BLE-Gamepad-Client@^0.1.0
```

# Example usage
* Add the following code to your project.
```cpp
#include <Arduino.h>
#include <BLEGamepadClient.h>

void onControlsUpdate(ControlsEvent& e) {
  Serial.printf("lx: %.2f, ly: %.2f\n", e.leftStickX, e.leftStickY);
}

Controller controller;

void setup(void) {
  Serial.begin(115200);
  controller.onControlsUpdate(onControlsUpdate);
  controller.begin();
}

void loop() {
  delay(1000);
}
```

* Using the [pio run](https://docs.platformio.org/en/latest/core/userguide/cmd_run.html#cmd-run) command, build the image.
```shell
pio run
```
* Upload the image to the connected board and start monitoring the serial port.
```shell
pio run -t upload -t monitor
```
* Call to `controller.begin()` will automatically start scanning until a controller is successfully connected. Connect the controller to the board using the controller's pairing button.
* After connecting a controller to the board, you should observe changing values of `lx` and `ly` in response to movement of the gamepad's left analog stick.

# Supported gamepads

## Xbox One Wireless Controller (model 1708)
Pairing instructions are the same as for model 1914. If controller is not pairing, you probably need to
update the controller's firmware to version 5.x using [these instructions](https://support.xbox.com/en-US/help/hardware-network/controller/update-xbox-wireless-controller). 

## Xbox Series S/X Wireless Controller (model 1914)
Pairing instructions:
* Turn on your controller by pressing the Xbox button.
* Press and hold the controller’s Pair button for 3 seconds, then release.
