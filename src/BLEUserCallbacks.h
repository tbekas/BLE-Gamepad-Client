#pragma once
#include <BLEAutoScan.h>
#include <BLEBaseController.h>
#include <freertos/queue.h>
#include <cstdint>
#include "messages.h"

class BLEUserCallbacks {
  public:
  BLEUserCallbacks(BLEAutoScan& autoScan, QueueHandle_t& userCallbackQueue);
 private:
  static void _callbackQueueConsumerTaskFn(void* pvParameters);

  QueueHandle_t& _userCallbackQueue;
  TaskHandle_t _userCallbackQueueConsumerTask;
  BLEAutoScan& _autoScan;
};
