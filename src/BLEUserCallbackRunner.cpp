#include "BLEUserCallbackRunner.h"

#include "logger.h"

BLEUserCallbackRunner::BLEUserCallbackRunner(BLEAutoScan& autoScan, QueueHandle_t& userCallbackQueue)
    : _userCallbackQueue(userCallbackQueue), _userCallbackQueueConsumerTask(nullptr), _autoScan(autoScan) {
  _userCallbackQueue = xQueueCreate(10, sizeof(BLEUserCallback));
  configASSERT(_userCallbackQueue);

  xTaskCreate(_userCallbackQueueConsumerTaskFn, "_userCallbackQueueConsumerTaskFn", 10000, this, 0,
              &_userCallbackQueueConsumerTask);
  configASSERT(_userCallbackQueueConsumerTask);
}
BLEUserCallbackRunner::~BLEUserCallbackRunner() {
  if (_userCallbackQueueConsumerTask != nullptr) {
    vTaskDelete(_userCallbackQueueConsumerTask);
    _userCallbackQueueConsumerTask = nullptr;
  }

  if (_userCallbackQueue != nullptr) {
    vQueueDelete(_userCallbackQueue);
    _userCallbackQueue = nullptr;
  }
}

void BLEUserCallbackRunner::_userCallbackQueueConsumerTaskFn(void* pvParameters) {
  auto* self = static_cast<BLEUserCallbackRunner*>(pvParameters);

  while (true) {
    BLEUserCallback msg{};
    if (xQueueReceive(self->_userCallbackQueue, &msg, portMAX_DELAY) != pdTRUE) {
      BLEGC_LOGE("Failed to receive user callback message");
      return;
    }

    switch (msg.kind) {
      case BLEUserCallbackKind::ControllerConnecting:
        msg.pCtrl->callOnConnecting();
        break;
      case BLEUserCallbackKind::ControllerConnectionFailed:
        msg.pCtrl->callOnConnectionFailed();
        break;
      case BLEUserCallbackKind::ControllerConnected:
        msg.pCtrl->callOnConnected();
        break;
      case BLEUserCallbackKind::ControllerDisconnected:
        msg.pCtrl->callOnDisconnected();
        break;
      case BLEUserCallbackKind::ScanStarted:
        self->_autoScan.callOnScanStarted();
        break;
      case BLEUserCallbackKind::ScanStopped:
        self->_autoScan.callOnScanStopped();
        break;
    }
  }
}
