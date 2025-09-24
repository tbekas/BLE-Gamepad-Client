#pragma once

#include <NimBLEDevice.h>
#include <functional>

template <typename T>
using OnUpdate = std::function<void(T& value)>;

template <typename T>
class BLEValueReceiver {
 public:
  BLEValueReceiver();
  ~BLEValueReceiver();

  /**
   * @brief Read the latest event from the connected controller.
   * @param[out] event Reference to the event instance where the data will be written.
   */
  void read(T& event);

  /**
   * @brief Sets the callback to be invoked when the controller sends a new event.
   * @param callback Reference to the callback function.
   */
  void onUpdate(const OnUpdate<T>& callback);

 protected:
  bool init(NimBLERemoteCharacteristic* pChar);

 private:
  struct Store {
    T event{};
  };
  static void _callbackTaskFn(void* pvParameters);
  void _handleNotify(NimBLERemoteCharacteristic* pChar, uint8_t* pData, size_t dataLen, bool isNotify);

  TaskHandle_t _callbackTask;
  SemaphoreHandle_t _storeMutex;
  Store _store;
  OnUpdate<T> _onUpdateCallback;
  bool _onUpdateCallbackSet;
};
