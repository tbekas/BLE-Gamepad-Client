#include <Arduino.h>
#include <BLEGamepadClient.h>

Controller controller;

void setup(void) {
  Serial.begin(115200);
  controller.begin();
}

void loop() {
  ControlsEvent e;

  if (controller.isConnected()) {
    controller.readControls(e);

    Serial.printf("lx: %.2f, ly: %.2f, rx: %.2f, ry: %.2f\n",
      e.leftStickX, e.leftStickY, e.rightStickX, e.rightStickY);
  }

  delay(100);
}
