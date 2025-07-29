#include <Arduino.h>
#include <BLEControllerRegistry.h>


// As shown in the example below, specifying a MAC address for Controller instances is optional.
// However, doing so ensures that a physical controller with the specified MAC address will be
// assigned to the corresponding instance of the Controller class when it connects to the board.
//
// If no MAC address is specified, controllers will be assigned based on the order in which they connect.
// In this case, the first connected physical controller will be assigned to `controller1` (since it is
// the first instance to call `.begin()`), regardless of its physical address.


Controller controller1;
Controller controller2("5f:7a:30:78:22:2a");

void setup(void) {
  Serial.begin(115200);
  controller1.begin();
  controller2.begin();
}

void loop() {
  ControlsEvent e;

  if (controller1.isConnected()) {
    controller1.readControls(e);

    Serial.printf("controller1 lx: %.2f, ly: %.2f, rx: %.2f, ry: %.2f\n",
      e.leftStickX, e.leftStickY, e.rightStickX, e.rightStickY);
  }

  if (controller2.isConnected()) {
    controller2.readControls(e);

    Serial.printf("controller2 lx: %.2f, ly: %.2f, rx: %.2f, ry: %.2f\n",
      e.leftStickX, e.leftStickY, e.rightStickX, e.rightStickY);
  }

  delay(100);
}
