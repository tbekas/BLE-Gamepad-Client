# BLE-Gamepad-Client
Enables connection of BLE gamepads to ESP32 boards. Currently, only the Xbox Wireless Controller is supported.

# PlatformIO dependency
* Open [platformio.ini](https://docs.platformio.org/en/latest/projectconf/index.html), a project configuration file located in the root of PlatformIO project.
* Add the following line to the [lib_deps](https://docs.platformio.org/en/latest/projectconf/sections/env/options/library/lib_deps.html) option of `[env:]` section:
```
tbekas/BLE-Gamepad-Client@^0.1.0
```
* Build a project, PlatformIO will automatically install dependencies.

# Example usage
* Add the following code to your project.
```cpp
#include <Arduino.h>
#include <BLEGamepadClient.h>

void onUpdate(ControlsEvent& e) {
  Serial.printf("lx: %f, ly: %f\n", e.lx, e.ly);
}

void onConnected(Controller& controller) {
  controller.controls().subscribe(onUpdate);
}

void setup(void) {
  Serial.begin(115200);
  GamepadClient.setConnectedCallback(onConnected);
  GamepadClient.begin();
}

void loop() {
  delay(1000);
}
```

* Using the [pio run](https://docs.platformio.org/en/latest/core/userguide/cmd_run.html#cmd-run) command, build the image, upload it to the connected board, and start monitoring the serial port.
```
pio run -t upload -t monitor
```
* By default, calling `GamepadClient.begin()` will start scanning until a controller is successfully connected. Connect the controller using the pairing button.

* After connecting a controller to the board, you should observe changing values of `lx` and `ly` in response to movement of the gamepad's left analog stick.

# Supported gamepads

## Xbox Wireless Controller (model 1914)
Pairing instructions:
* Turn on your controller by pressing the Xbox button.
* Press and hold the controller’s Pair button for 3 seconds, then release.