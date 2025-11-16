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

# Connecting more than 3 devices

By default, the [NimBLE](https://github.com/h2zero/NimBLE-Arduino) library allows up to 3 simultaneous BLE connections.
You can increase this limit by setting the `CONFIG_BT_NIMBLE_MAX_CONNECTIONS` configuration option to a higher value.

ESP32 boards support up to **9 concurrent BLE connections**.
