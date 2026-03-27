# BLE-Gamepad-Client — Developer Guide

## Project overview

Arduino/PlatformIO library (C++17) for connecting BLE gamepads to ESP32 boards. The core design goal is **simplicity of use**: users instantiate a controller, call `begin()`, and either poll with `read()` or register callbacks. All BLE complexity is hidden behind this surface.

- **Platform**: ESP32 only (`architectures=esp32`), Arduino framework
- **Dependency**: `h2zero/NimBLE-Arduino ^2.3.2`
- **License**: Apache-2.0

## Build & CI

Build is tested exclusively with PlatformIO. No local build script exists; use the CI workflow as a reference.

```bash
# Minimal platformio.ini for development
[env:esp32dev]
platform = espressif32
framework = arduino
board = esp32dev
build_unflags = -std=gnu++14
```

CI (`build.yml`) tests five examples across three boards: `esp32dev`, `esp32s3`, `esp32c3`. Docs are built with MkDocs + Material (`mkdocs==1.6.1`, `mkdocs-material==9.7.0`). Arduino Lint runs on every PR.

## Architecture

### Component map

```
BLEGamepadClient          static façade / entry point, initializes NimBLE
├── BLEAutoScan           FreeRTOS task: manages scan lifecycle (high-duty → low-duty)
├── BLEControllerRegistry manages registered controllers, owns NimBLE client callbacks
│   └── ClientCallbacksImpl  NimBLE callbacks → internal ClientEvent queue
└── BLEUserCallbackRunner drains userCallbackQueue, fires user-facing callbacks

BLEAbstractController     connection state machine, address allocation (lock-free CAS)
└── BLEBaseController<T>  CRTP layer: typed onConnecting/onConnected/onDisconnected callbacks
    └── XboxController / SteamController
        ├── BLEValueReceiver<XboxControlsState>   subscribes to HID notifications
        ├── BLEValueReceiver<XboxBatteryState>
        └── BLEValueWriter<XboxVibrationsCommand> writes HID output reports
```

### Threading model

There are four FreeRTOS tasks plus the NimBLE stack's own tasks:

| Task | Purpose |
|---|---|
| `_autoScanTask` | Receives task notifications, starts/stops scans |
| `_clientEventConsumerTask` | Serialises NimBLE client events (pinned to `CONFIG_BT_NIMBLE_PINNED_TO_CORE`) |
| `BLEUserCallbackRunner` task | Drains `_userCallbackQueue`, calls all user-facing callbacks |
| `_callbackTask` (one per `BLEValueReceiver`) | Fires `onValueChanged` callbacks off the BLE notify thread |

User callbacks are always invoked from the `BLEUserCallbackRunner` task or a `BLEValueReceiver` callback task — never directly from a NimBLE callback. This means user callback code does not need to be ISR-safe, but it should not block for long.

### Connection sequence

`begin()` → registered → auto-scan starts → device discovered → `tryAllocate()` (lock-free) → NimBLE client created → `connect()` → `onConnect` (NimBLE) → `secureConnection()` → `onAuthenticationComplete` bonded → `hidInit()` + controller `init()` → `markConnected()` → `onConnected` user callback.

On disconnect: `onDisconnect` (NimBLE) → `deinit()` → `tryDeallocate()` → `deleteClient()` → `onDisconnected` user callback → auto-scan restarts.

### Auto-scan

Scans in two phases after `begin()`:
1. **High-duty** (default 60 s): aggressive window/interval (10 ms / 10 ms), active scan
2. **Low-duty** (default 240 s): power-efficient (1280 ms / 15 ms), passive scan

`BLEAutoScan::notify()` restarts scanning manually if the scan window expired before all slots filled.

## Conventions

### Naming

- Classes: `PascalCase`, infrastructure prefixed `BLE`, user-facing types are descriptive (`XboxController`, `XboxControlsState`)
- Private members: `_camelCase`
- Public methods: `camelCase`
- Config overrides: `CONFIG_BT_BLEGC_*` preprocessor defines (see `src/config.h`)
- Utility namespace: `blegc::` (BLE GATT UUIDs, appearance constants, helper functions)
- Logging macros: `BLEGC_LOGV/D/I/W/E`

### Value types (state & command structs)

All state and command structs extend `BLEBaseValue`:

- **State structs** (`*ControlsState`, `*BatteryState`) implement `decode(uint8_t[], size_t)` → `BLEDecodeResult` and equality operators
- **Command structs** (`*VibrationsCommand`) implement `encode(size_t&, uint8_t[], size_t)` → `BLEEncodeResult`
- Analog axes / triggers are normalised to `float`: sticks `[-1.0, 1.0]`, triggers/battery `[0.0, 1.0]`
- `controllerAddress` (peer `NimBLEAddress`) is set by `BLEValueReceiver` on subscribe

### Headers

- `#pragma once` everywhere (no include guards)
- All public types are re-exported through `src/BLEGamepadClient.h`, which is the single include for users

## Adding a new controller

1. Create `src/<name>/` directory.
2. Add a `*ControlsState` struct extending `BLEBaseValue` — implement `decode()`, `operator==`, `operator!=`.
3. Add a `*Controller` class extending `BLEBaseController<ControllerType>` and `BLEValueReceiver<ControlsState>`:
   - Override `isSupported(const NimBLEAdvertisedDevice*)` — identify device by name / appearance / manufacturer ID
   - Override `init()` — find characteristics with `blegc::findNotifiableCharacteristic()`, call `BLEValueReceiver<T>::init()`
   - Override `deinit()` — clean up (usually `return true` is fine)
4. Add explicit template instantiation at the bottom of `src/BLEValueReceiver.cpp`:
   ```cpp
   template class BLEValueReceiver<YourControlsState>;
   ```
5. Export the controller header from `src/BLEGamepadClient.h`.
6. Add a `<Name>_ReadingControls` example and include it in the CI build matrix in `.github/workflows/build.yml`. All examples are prefixed with the controller name (e.g. `Xbox_`, `Steam_`).

If the controller supports write (e.g. vibrations), also extend `BLEValueWriter<CommandType>` and add a corresponding `*Command` struct with `encode()`.

## Configuration reference

All defaults live in `src/config.h` and can be overridden via `build_flags` in `platformio.ini`:

| Define | Default | Description |
|---|---|---|
| `CONFIG_BT_BLEGC_HIGH_DUTY_SCAN_DURATION_MS` | `60000` | High-duty scan duration |
| `CONFIG_BT_BLEGC_LOW_DUTY_SCAN_DURATION_MS` | `240000` | Low-duty scan duration |
| `CONFIG_BT_BLEGC_HIGH_DUTY_SCAN_INTERVAL_MS` | `10` | High-duty scan interval |
| `CONFIG_BT_BLEGC_HIGH_DUTY_SCAN_WINDOW_MS` | `10` | High-duty scan window |
| `CONFIG_BT_BLEGC_LOW_DUTY_SCAN_INTERVAL_MS` | `1280` | Low-duty scan interval |
| `CONFIG_BT_BLEGC_LOW_DUTY_SCAN_WINDOW_MS` | `15` | Low-duty scan window |
| `CONFIG_BT_BLEGC_CONN_TIMEOUT_MS` | `15000` | Connection timeout |
| `CONFIG_BT_BLEGC_DEVICE_NAME` | `ESP.getChipModel()` | BLE device name advertised |
| `CONFIG_BT_BLEGC_POWER_DBM` | `0` | TX power in dBm |
| `CONFIG_BT_BLEGC_SECURITY_AUTH` | bond+MITM+SC | NimBLE auth flags |
| `CONFIG_BT_BLEGC_SECURITY_IO_CAP` | `BLE_HS_IO_NO_INPUT_OUTPUT` | Pairing I/O capability |
| `CONFIG_BT_BLEGC_LOG_LEVEL` | `CORE_DEBUG_LEVEL` or `1` | Log verbosity |
| `CONFIG_BT_BLEGC_LOG_BUFFER_ENABLED` | `0` | Enable raw HID report logging |
| `CONFIG_BT_BLEGC_WRITER_BUFFER_MAX_CAPACITY` | `1024` | Write buffer size (bytes) |

NimBLE can also be initialised manually before `begin()` — the library skips its own `NimBLEDevice::init()` if NimBLE is already running.

## Key design constraints

- **`BLEGamepadClient` is never instantiated** — it is a fully static class. Construction is explicitly deleted.
- **`BLEValueReceiver.cpp` uses explicit template instantiation** — add a new `template class BLEValueReceiver<T>` line when adding a new state type, otherwise you will get linker errors.
- **Controller allocation is lock-free CAS** (`std::atomic_uint64_t _address`) — `tryAllocate` / `tryDeallocate` must not be called with locks held.
- **Multiple controllers of the same type are supported** — the registry prefers allocating the controller that was last connected to a given address, then unconnected controllers, then any other.
- **`deleteBonds = true` by default** in `BLEGamepadClient::init()` — bonding info is cleared on every power-on to avoid stale pairings. Pass `false` to `init()` or call `NimBLEDevice::init()` manually to retain bonds.
