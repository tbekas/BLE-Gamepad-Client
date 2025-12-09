#pragma once
#include <BLEAutoScan.h>
#include <BLEBaseController.h>
#include <freertos/queue.h>

class BLEUserCallbackRunner {
  public:
  BLEUserCallbackRunner(BLEAutoScan& autoScan, QueueHandle_t& userCallbackQueue);
  ~BLEUserCallbackRunner();
 private:
  static void _userCallbackQueueConsumerTaskFn(void* pvParameters);

  QueueHandle_t& _userCallbackQueue;
  TaskHandle_t _userCallbackQueueConsumerTask;
  BLEAutoScan& _autoScan;
};
