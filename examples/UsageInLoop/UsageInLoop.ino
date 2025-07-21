#include <Arduino.h>
#include <BLEGamepadClient.h>

void setup(void) {
  Serial.begin(115200);
  BLEGamepadClient.begin();
}

void loop() {
  // Note the usage of a reference type `Controller&`
  for (Controller& ctrl : BLEGamepadClient.getControllers()) {
    if (ctrl.isConnected()) {
      ControlsEvent e;
      ctrl.controls().read(e);

      Serial.printf("lx: %.2f, ly: %.2f, rx: %.2f, ry: %.2f\n",
        e.leftStickX, e.leftStickY, e.rightStickX, e.rightStickY);
    }
  }
  delay(100);
}
