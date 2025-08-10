#include <Arduino.h>
#include <BLEGamepadClient.h>

BLEController controller;

void setup(void) {
  Serial.begin(115200);
  controller.begin();
}

void loop() {
  if (controller.isConnected()) {
    BLEControlsEvent e;
    controller.readControls(e);

    Serial.printf("lx: %.2f, ly: %.2f, rx: %.2f, ry: %.2f\n",
      e.leftStickX, e.leftStickY, e.rightStickX, e.rightStickY);
  } else {
    Serial.println("controller not connected");
  }
  delay(100);
}
