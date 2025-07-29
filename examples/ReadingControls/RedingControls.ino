#include <Arduino.h>
#include <BLEController.h>

BLEController controller;

void setup(void) {
  Serial.begin(115200);
  controller.begin();
}

void loop() {
  BLEControlsEvent e;

  if (controller.isConnected()) {
    controller.readControls(e);

    Serial.printf("lx: %.2f, ly: %.2f, rx: %.2f, ry: %.2f\n",
      e.leftStickX, e.leftStickY, e.rightStickX, e.rightStickY);
  }

  delay(100);
}
