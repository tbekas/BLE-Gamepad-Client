#pragma once

#include <NimBLEDevice.h>

template <typename T>
class BLEValueWriter {
 public:
  BLEValueWriter();
  ~BLEValueWriter();

 /**
  * @brief Send the command to the connected controller.
  * @param cmd Command to send.
  */
  void write(T& cmd);

 protected:
  bool init(NimBLERemoteCharacteristic* pChar);

 private:
  struct Store {
    uint8_t* pBuffer{};
    uint8_t* pSendBuffer{};
    size_t used{};
    size_t capacity{};
  };
  static void _sendDataFn(void* pvParameters);

  NimBLERemoteCharacteristic* _pChar;
  TaskHandle_t _sendDataTask;
  SemaphoreHandle_t _storeMutex;
  Store _store;
};
