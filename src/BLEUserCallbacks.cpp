#include "BLEUserCallbacks.h"

BLEUserCallbacks::BLEUserCallbacks(BLEAutoScan& autoScan, QueueHandle_t& userCallbackQueue)
    : _userCallbackQueue(userCallbackQueue), _userCallbackQueueConsumerTask(nullptr), _autoScan(autoScan) {
  _userCallbackQueue = xQueueCreate(10, sizeof(BLEUserCallback));
  configASSERT(_userCallbackQueue);

  xTaskCreate(_callbackQueueConsumerTaskFn, "_callbackQueueConsumerTask", 10000, this, 0, &_userCallbackQueueConsumerTask);
  configASSERT(_userCallbackQueueConsumerTask);
}

void BLEUserCallbacks::_callbackQueueConsumerTaskFn(void* pvParameters) {
  auto* self = static_cast<BLEUserCallbacks*>(pvParameters);

  while (true) {
    BLEUserCallback msg{};
    if (xQueueReceive(self->_userCallbackQueue, &msg, portMAX_DELAY) != pdTRUE) {
      BLEGC_LOGE("Failed to receive user callback message");
      return;
    }

    switch (msg.kind) {
      case BLEUserCallbackKind::BLEControllerConnecting:
        msg.pCtrl->callOnConnecting();
      case BLEUserCallbackKind::BLEControllerConnectionFailed:
        msg.pCtrl->callOnConnectionFailed();
        break;
      case BLEUserCallbackKind::BLEControllerConnected:
        msg.pCtrl->callOnConnected();
        break;
      case BLEUserCallbackKind::BLEControllerDisconnected:
        msg.pCtrl->callOnDisconnected();
        break;
      case BLEUserCallbackKind::BLEScanStarted:
        self->_autoScan.callOnScanStarted();
        break;
      case BLEUserCallbackKind::BLEScanStopped:
        self->_autoScan.callOnScanStopped();
        break;
    }
  }
}
