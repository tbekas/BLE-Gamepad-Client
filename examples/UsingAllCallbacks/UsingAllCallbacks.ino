/*
* This program demonstrates how to use callbacks to change the state of an RGB LED.
 *
 * scanning   -> blink the LED
 * connecting -> blink the LED more rapidly
 * connected  -> turn the LED on
 */

#include <Arduino.h>
#include <BLEGamepadClient.h>
#include "esp32-hal-rgb-led.h"

#if defined(LED_BUILTIN)
#define LED_PIN LED_BUILTIN
#else
#define LED_PIN 10  // you may need to change this
#endif

#define LED_BRIGHTNESS RGB_BRIGHTNESS // 0 - 255 range

BLEAutoScan *pAutoScan = BLEGamepadClient::getAutoScan();

XboxController controller;

TaskHandle_t blinkLedTask;

void blinkLed(uint32_t durationMs);
void turnLedOn();
void turnLedOff();

void onScanStarted() {
  Serial.println("scan started");
  blinkLed(500);
}
void onScanStopped() {
  Serial.println("scan stopped");
  turnLedOff();
}
void onConnecting(XboxController &ctrl) {
  Serial.println("connecting");
  blinkLed(100);
}
void onConnectionFailed(XboxController &ctrl) {
  Serial.println("connection failed");
  turnLedOff();
}
void onConnected(XboxController &ctrl) {
  Serial.println("connected");
  turnLedOn();
}
void onDisconnected(XboxController &ctrl) {
  Serial.println("disconnected");
  turnLedOff();
}

void onValueChanged(XboxControlsState &s) {
  Serial.printf("lx: %.2f, ly: %.2f, rx: %.2f, ry: %.2f\n",
    s.leftStickX, s.leftStickY, s.rightStickX, s.rightStickY);
}

void setup(void) {
  Serial.begin(115200);

  pAutoScan->onScanStarted(onScanStarted);
  pAutoScan->onScanStopped(onScanStopped);

  controller.begin();
  controller.onConnecting(onConnecting);
  controller.onConnectionFailed(onConnectionFailed);
  controller.onConnected(onConnected);
  controller.onDisconnected(onDisconnected);
  controller.onValueChanged(onValueChanged);
}

void loop() { delay(100); }

void blinkLedTaskFn(void *pvParameters) {
  uint32_t durationMs = reinterpret_cast<uint32_t>(pvParameters);

  while (true) {
    neopixelWrite(LED_PIN, 0, 0, LED_BRIGHTNESS);
    delay(durationMs);
    neopixelWrite(LED_PIN, 0, 0, 0);
    delay(durationMs);
  }
}

void stopBlinkLedTask() {
  if (blinkLedTask != nullptr) {
    vTaskDelete(blinkLedTask);
    blinkLedTask = nullptr;
  }
}

void blinkLed(uint32_t durationMs) {
  stopBlinkLedTask();

  xTaskCreate(blinkLedTaskFn, "blinkLedTask", 10000,
              reinterpret_cast<void *>(durationMs), 0, &blinkLedTask);
  configASSERT(blinkLedTask);
}

void turnLedOn() {
  stopBlinkLedTask();
  neopixelWrite(LED_PIN, 0, 0, LED_BRIGHTNESS);
}

void turnLedOff() {
  stopBlinkLedTask();
  neopixelWrite(LED_PIN, 0, 0, 0);
}
