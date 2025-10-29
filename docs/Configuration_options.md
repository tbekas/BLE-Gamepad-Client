# Configuration options

## General options

`CONFIG_BT_BLEGC_LOG_LEVEL`

Defines the log message level. If not defined, it will default to the same value as the Arduino core debug level.
Available values:

* `0` = NONE
* `1` = ERROR
* `2` = WARNING
* `3` = INFO
* `4` = DEBUG
* `5` = TRACE

**Default**: `CORE_DEBUG_LEVEL` or `0` (NONE) if `CORE_DEBUG_LEVEL` is not defined  
<br/>

`CONFIG_BT_BLEGC_LOGGER`

Logger function used to print log messages to the output.  
**Default**: `printf`  
<br/>

`CONFIG_BT_BLEGC_ENABLE_DEBUG_DATA`

Enables storing or printing data transferred to or from the device, including HID reports and the HID report map.

Available values:
* `0` - disabled
* `1` - enabled

**Default**: `0` (disabled)  
<br/>

`CONFIG_BT_BLEGC_WRITER_MAX_CAPACITY`

Maximum capacity, in bytes, of the internal buffer used to send data to the controller.  
**Default**: 1024  
<br/>

`CONFIG_BT_BLEGC_MAX_CONNECTION_SLOTS`

The maximum number of connection slots that can be created.  
**Default**: `CONFIG_BT_BLEGC_MAX_CONNECTION_SLOTS` (default 3) or 9 if `CONFIG_BT_BLEGC_MAX_CONNECTION_SLOTS` is not defined  
<br/>

`CONFIG_BT_BLEGC_SCAN_DURATION_MS`

Duration (in milliseconds) of a single scan. The next scan starts automatically after the previous one ends.  
Scanning stops once all initialized controllers are connected.  
**Default**: `30000` (30 seconds)  
<br/>

`CONFIG_BT_BLEGC_CONN_TIMEOUT_MS`

Timeout (in milliseconds) for establishing a connection with a peer.  
**Default**: `15000` (15 seconds)  
<br/>

---

## NimBLE initialization settings

`CONFIG_BT_BLEGC_DEVICE_NAME`

Name advertised by the device.  
**Default**: `"ESP32"`  
<br/>

`CONFIG_BT_BLEGC_POWER_DBM`

Transmission power in dBm.  
**Default**: `3`  
<br/>

`CONFIG_BT_BLEGC_SECURITY_IO_CAP`

Defines the local Input/Output capabilities of the device. Each option determines the pairing method:

* `BLE_HS_IO_KEYBOARD_ONLY` — Passkey pairing
* `BLE_HS_IO_DISPLAY_YESNO` — Numeric comparison pairing
* `BLE_HS_IO_NO_INPUT_OUTPUT` — Just works pairing

**Default**: `BLE_HS_IO_NO_INPUT_OUTPUT` (no screen, no keyboard)  
<br/>

`CONFIG_BT_BLEGC_SECURITY_AUTH`

Bitmap representing the required authentication modes for pairing. Available flags:

* `BLE_SM_PAIR_AUTHREQ_BOND` — Require storing keys for bonding
* `BLE_SM_PAIR_AUTHREQ_MITM` — Require Man-in-the-Middle protection
* `BLE_SM_PAIR_AUTHREQ_SC` — Use LE Secure Connections

**Default**: `BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM | BLE_SM_PAIR_AUTHREQ_SC`  
<br/>

---

## Manual NimBLE initialization

If the settings above are not sufficient for your use case, you can initialize the NimBLE stack manually **before**
initializing any controller instance.

```cpp
#include <Arduino.h>
#include <BLEControllerRegistry.h>

Controller controller;

void setup() {
    NimBLEDevice::init("My device");
    NimBLEDevice::setPower(3);                                  // +3 dBm
    NimBLEDevice::setSecurityAuth(true, true, true);            // bonding, MITM protection, secure connections
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);  // no screen, no keyboard
    
    controller.begin();
}
```

<br/>
