# Auto Scan

## How it works

The auto-scan feature (enabled by default) simplifies connecting BLE controllers by automatically managing the scanning
process in the background. It **starts scanning whenever there are any controller instances that are not
yet connected**, and stops scanning once all instances are connected. Whenever a controller disconnects scanning is
started again.

## State diagram

Below is the visual representation of state transitions.

```mermaid
stateDiagram-v2
    direction TB
    Idle
    Scanning
    Connecting
    Connected
    ConnectingFailed: Connecting Failed
    Disconnected
    
    Scanning --> Connecting : device discovered
    Connecting --> ConnectingFailed : failure
    Connecting --> Connected : success
    Connected --> Disconnected : device lost or disconnect()
    Disconnected --> Scanning
    ConnectingFailed --> Scanning
    Idle --> Scanning : begin() or notify()
    Scanning --> Idle : scan timeout or end()
```
