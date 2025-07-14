#include <Arduino.h>
#include <BLEGamepadClient.h>

void setup(void) {
  Serial.begin(115200);
  GamepadClient.begin();
}

void loop() {
  // Note the usage of a reference type `Controller&`
  for (Controller& ctrl : GamepadClient.getControllers()) {
    if (ctrl.isConnected() && ctrl.controls().isUpdated()) {
      ControlsEvent e = ctrl.controls().read();

      Serial.printf("lx: %.2f, ly: %.2f, rx: %.2f, ry: %.2f\n", e.lx, e.ly, e.rx, e.ry);
    }
  }
  delay(100);
}