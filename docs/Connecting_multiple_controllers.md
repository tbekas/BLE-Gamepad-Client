# Connecting multiple controllers

Connecting two or more controllers is as simple as creating multiple controller instances and initializing them
using `begin()`.

```cpp
#include <BLEGamepadClient.h>

SteamController controller1;
XboxController controller2;
XboxController controller3;

void setup(void) {
  controller1.begin();
  controller2.begin();
  controller3.begin();
}

void loop() {
  // Code omitted for brevity
}
```

If no MAC address is specified, controllers are assigned based on the order in which they connect. In this case, the
first connected xbox controller will be assigned to `controller2` (as it calls `begin()` before `controller3` does).

If you need a specific physical controller to be assigned to a specific controller instance, you can provide its
MAC address when constructing the instance.

```cpp
SteamController controller1;
XboxController controller2("5f:7a:30:78:22:2a");
XboxController controller3("6c:d8:d4:31:b0:5d");
```

This ensures that each physical controller is always mapped to the correct instance, regardless of the connection order.

If you don’t know the MAC address of your controller, you can use
the [nRF Connect for Mobile](https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-mobile) app to scan
nearby BLE devices.

Alternatively, you can register an `onConnect` callback on the controller instance and log the
controller’s MAC address to the serial output when it connects.

```cpp
#include <Arduino.h>
#include <BLEGamepadClient.h>

void onConnect(XboxController& ctrl) {
  Serial.printf("controller connected, address: %s\n", ctrl.getAddress().toString().c_str());
}

XboxController controller;

void setup(void) {
  Serial.begin(115200);
  controller.begin();
  controller.onConnect(onConnect);
}

void loop() {
  // Code omitted for brevity
}
```

# Connecting more than 3 devices

By default, the [NimBLE](https://github.com/h2zero/NimBLE-Arduino) library allows up to 3 simultaneous BLE connections.
You can increase this limit by setting the `CONFIG_BT_NIMBLE_MAX_CONNECTIONS` configuration option to a higher value.

ESP32 boards support up to **9 concurrent BLE connections**.
