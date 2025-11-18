#pragma once

#include <NimBLEDevice.h>
#include <utils.h>
#include <atomic>
#include <functional>
#include <memory>

class BLEAbstractController {
 public:
  virtual ~BLEAbstractController() = default;
  explicit BLEAbstractController();

  void begin();
  void end();
  NimBLEAddress getAddress() const;
  NimBLEAddress getLastAddress() const;
  bool isConnected() const;
  void disconnect();

  friend class BLEControllerRegistry;

 protected:
  const static NimBLEAddress _nullAddress;

  NimBLEClient* getClient() const;
  bool tryAllocate(NimBLEAddress address);
  bool tryDeallocate();
  bool isAllocated() const;
  void markCompletedDeregistration();
  void markConnected();
  void markDisconnected();
  bool isPendingDeregistration() const;
  bool hidInit(NimBLEClient* pClient);

  virtual void callOnConnect() = 0;
  virtual void callOnDisconnect() = 0;
  virtual bool isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) = 0;
  virtual bool init(NimBLEClient* pClient) = 0;
  virtual bool deinit() = 0;

  std::atomic_bool _pendingDeregistration;
  std::atomic<const NimBLEAddress*> _address;
  bool _connected;
  NimBLEAddress _lastAddress;
  blegc::BLEDeviceInfo _deviceInfo;
};

template <typename T>
class BLEBaseController : public BLEAbstractController {
 public:
  explicit BLEBaseController() : _onConnect([](T&) {}), _onDisconnect([](T&) {}) {}

  /**
   * @brief Sets the callback to be invoked when the controller connects.
   * @param callback Reference to a callback function.
   */
  void onConnect(const std::function<void(T&)>& callback) { _onConnect = callback; }

  /**
   * @brief Sets the callback to be invoked when the controller disconnects.
   * @param callback Reference to the callback function.
   */
  void onDisconnect(const std::function<void(T&)>& callback) { _onDisconnect = callback; }

 protected:
  void callOnConnect() override { _onConnect(*static_cast<T*>(this)); }
  void callOnDisconnect() override { _onDisconnect(*static_cast<T*>(this)); }

 private:
  std::function<void(T&)> _onConnect;
  std::function<void(T&)> _onDisconnect;
};
