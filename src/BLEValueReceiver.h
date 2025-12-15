#pragma once

#include <NimBLEDevice.h>
#include <functional>

template <typename T>
using OnValueChanged = std::function<void(T& value)>;

template <typename T>
class BLEValueReceiver {
 public:
  BLEValueReceiver();
  ~BLEValueReceiver();

  /**
   * @brief Read the latest value from the connected controller.
   * @param[out] value Pointer to the value instance where the data will be written.
   */
  void read(T* value);

  /**
   * @brief Sets a callback that is invoked whenever the value changes.
   * @param callback The function to call when a new value is received that differs from the previous one.
   */
  void onValueChanged(const OnValueChanged<T>& callback);

 protected:
  bool init(NimBLERemoteCharacteristic* pChar);

 private:
  struct Store {
    T value{};
  };
  static void _callbackTaskFn(void* pvParameters);
  void _handleNotify(NimBLERemoteCharacteristic* pChar, uint8_t* pData, size_t dataLen, bool isNotify);

  TaskHandle_t _callbackTask;
  SemaphoreHandle_t _storeMutex;
  Store _store;
  OnValueChanged<T> _onValueChangedCallback;
  bool _onValueChangedCallbackSet;
};
