/**
 * More about connecting multiple controllers:
 * https://github.com/tbekas/BLE-Gamepad-Client/blob/main/docs/Connecting_multiple_controllers.md
 */

#include <Arduino.h>
#include <BLEGamepadClient.h>

XboxController controller1;
XboxController controller2("5f:7a:30:78:22:2a"); // MAC address is an optional parameter

void setup(void) {
  Serial.begin(115200);
  controller1.begin();
  controller2.begin();
}

void loop() {
  XboxControlsEvent e;

  if (controller1.isConnected()) {
    controller1.read(&e);

    Serial.printf("controller1 lx: %.2f, ly: %.2f, rx: %.2f, ry: %.2f\n",
      e.leftStickX, e.leftStickY, e.rightStickX, e.rightStickY);
  } else {
    Serial.println("controller1 not connected");
  }

  if (controller2.isConnected()) {
    controller2.read(&e);

    Serial.printf("controller2 lx: %.2f, ly: %.2f, rx: %.2f, ry: %.2f\n",
      e.leftStickX, e.leftStickY, e.rightStickX, e.rightStickY);
  } else {
    Serial.println("controller2 not connected");
  }

  delay(100);
}
