#pragma once

#include <NimBLEDevice.h>
#include <utils.h>

class BLEAbstractController {
 public:
  virtual ~BLEAbstractController() = default;
  explicit BLEAbstractController(NimBLEAddress allowedAddress);

  void begin();
  void end();
  NimBLEAddress getAddress() const;
  NimBLEAddress getLastAddress() const;
  NimBLEAddress getAllowedAddress() const;
  bool isConnected() const;
  void disconnect();

  friend class BLEControllerRegistry;

 protected:
  NimBLEClient* getClient() const;
  void setAddress(NimBLEAddress address);
  void setLastAddress(NimBLEAddress address);
  bool isAllocated() const;
  void markPendingDeregistration();
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

  volatile bool _pendingDeregistration;
  bool _connected;
  NimBLEAddress _address;
  NimBLEAddress _allowedAddress;
  NimBLEAddress _lastAddress;
  blegc::BLEDeviceInfo _deviceInfo;
};

template <typename T>
class BLEBaseController : public BLEAbstractController {
 public:
  explicit BLEBaseController(const NimBLEAddress& allowedAddress)
      : BLEAbstractController(allowedAddress), _onConnect([](T&) {}), _onDisconnect([](T&) {}) {}

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
