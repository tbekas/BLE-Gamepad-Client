# Connecting multiple controllers

Connecting two or more controllers is as simple as creating multiple `BLEController` instances and initializing them
using `BLEController::begin()`.

```cpp
#include <BLEController.h>

BLEController controller1;
BLEController controller2;

void setup(void) {
  controller1.begin();
  controller2.begin();
}

void loop() {
  // Code omitted for brevity
}
```

If no MAC address is specified, controllers are assigned based on the order in which they connect.
In this case, the first connected physical controller will be assigned to `controller1` (as it is the first instance to
call `begin()`), and so on.

If you need a specific physical controller to be assigned to a specific `BLEController` instance, you can provide its
MAC address when constructing the instance.

```cpp
BLEController controller1("5f:7a:30:78:22:2a");
BLEController controller2("6c:d8:d4:31:b0:5d");
```

This ensures that each physical controller is always mapped to the correct instance, regardless of the connection order.

If you don’t know the MAC address of your controller, you can use
the [nRF Connect for Mobile](https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-mobile) app to scan
nearby BLE devices.
Alternatively, you can enable debug logging by setting `CORE_DEBUG_LEVEL` to `4`, and check the MAC address in the
serial logs during the connection process.

# Connecting more than 3 devices

By default, the [NimBLE](https://github.com/h2zero/NimBLE-Arduino) library allows up to 3 simultaneous BLE connections.
You can increase this limit by setting the `CONFIG_BT_NIMBLE_MAX_CONNECTIONS` configuration option to a higher value.

ESP32 boards support up to **9 concurrent BLE connections**.
