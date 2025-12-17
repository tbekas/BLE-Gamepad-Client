# Configuration options

## General options

### `CONFIG_BT_BLEGC_LOG_LEVEL`

Defines the log message level. If not defined, it will default to the same value as the Arduino core debug level.
Available values:

* `0` = NONE
* `1` = ERROR
* `2` = WARNING
* `3` = INFO
* `4` = DEBUG
* `5` = VERBOSE

**Default**: `CORE_DEBUG_LEVEL` or `1` (ERROR) if `CORE_DEBUG_LEVEL` is not defined  
<br/>

### `CONFIG_BT_BLEGC_LOG_BUFFER_ENABLED`

Enables storing or printing data transferred to or from the device, including HID reports and the HID report map.

Available values:

* `0` - disabled
* `1` - enabled

**Default**: `0` (disabled)  
<br/>

### `CONFIG_BT_BLEGC_WRITER_BUFFER_MAX_CAPACITY`

Maximum capacity, in bytes, of the internal buffer used to send data to the controller.  
**Default**: 1024  
<br/>

### `CONFIG_BT_BLEGC_HIGH_DUTY_SCAN_DURATION_MS`

Duration, in milliseconds, of the high-duty scan phase. The high-duty scan runs first and is automatically followed by a
low-duty scan.  
**Default**: `60000` (60 seconds)  
<br/>

### `CONFIG_BT_BLEGC_HIGH_DUTY_SCAN_INTERVAL_MS`

Time, in milliseconds, between the start of two consecutive scan windows during the high-duty scan phase.  
**Default**: `10` (10 millis)  
<br/>

### `CONFIG_BT_BLEGC_HIGH_DUTY_SCAN_WINDOW_MS`

Duration, in milliseconds, of each scan window during the high-duty scan phase.  
**Default**: `10` (10 millis)  
<br/>

### `CONFIG_BT_BLEGC_HIGH_DUTY_SCAN_ACTIVE`

Enables or disables active scanning during the high-duty scan phase. When active scanning is enabled, scan response is
requested from advertisers.

Available values:

* `0` - disabled
* `1` - enabled

**Default**: `1` (enabled)  
<br/>

### `CONFIG_BT_BLEGC_LOW_DUTY_SCAN_DURATION_MS`

Duration, in milliseconds, of the low-duty scan phase. The low-duty scan starts immediately after the high-duty scan
completes.  
**Default**: `240000` (240 seconds)  
<br/>

### `CONFIG_BT_BLEGC_LOW_DUTY_SCAN_INTERVAL_MS`

Time, in milliseconds, between the start of two consecutive scan windows during the low-duty scan phase.  
**Default**: `1280` (1280 millis)  
<br/>

### `CONFIG_BT_BLEGC_LOW_DUTY_SCAN_WINDOW_MS`

Duration, in milliseconds, of each scan window during the low-duty scan phase.  
**Default**: `15` (15 millis)  
<br/>

### `CONFIG_BT_BLEGC_LOW_DUTY_SCAN_ACTIVE`

Enables or disables active scanning during the low-duty scan phase. When active scanning is enabled, scan response is
requested from advertisers.

Available values:

* `0` - disabled
* `1` - enabled

**Default**: `0` (disabled)  
<br/>

### `CONFIG_BT_BLEGC_CONN_TIMEOUT_MS`

Timeout (in milliseconds) for establishing a connection with a peer.  
**Default**: `15000` (15 seconds)  
<br/>

---

## NimBLE initialization settings

### `CONFIG_BT_BLEGC_DEVICE_NAME`

Name advertised by the device.  
**Default**: `ESP.getChipModel()`  
<br/>

### `CONFIG_BT_BLEGC_POWER_DBM`

Transmission power in dBm.  
**Default**: `0`  
<br/>

### `CONFIG_BT_BLEGC_SECURITY_IO_CAP`

Defines the local Input/Output capabilities of the device. Each option determines the pairing method:

* `BLE_HS_IO_KEYBOARD_ONLY` — Passkey pairing
* `BLE_HS_IO_DISPLAY_YESNO` — Numeric comparison pairing
* `BLE_HS_IO_NO_INPUT_OUTPUT` — Just works pairing

**Default**: `BLE_HS_IO_NO_INPUT_OUTPUT` (no screen, no keyboard)  
<br/>

### `CONFIG_BT_BLEGC_SECURITY_AUTH`

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
