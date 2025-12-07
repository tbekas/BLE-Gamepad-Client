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
  bool isConnecting() const;
  void disconnect();

  friend class BLEUserCallbackRunner;
  friend class BLEControllerRegistry;

 protected:
  enum class ConnectionState : uint8_t {
    Connecting,
    Connected,
    Disconnected,
  };

  const static NimBLEAddress _nullAddress;

  NimBLEClient* getClient() const;
  void setClient(NimBLEClient* pClient);
  bool tryAllocate(NimBLEAddress address);
  bool tryDeallocate();
  bool isAllocated() const;
  void markCompletedDeregistration();
  void markConnecting();
  void markConnected();
  void markDisconnected();
  bool isPendingDeregistration() const;
  bool hidInit(NimBLEClient* pClient);

  virtual void callOnConnecting() = 0;
  virtual void callOnConnectionFailed() = 0;
  virtual void callOnConnected() = 0;
  virtual void callOnDisconnected() = 0;
  virtual bool isSupported(const NimBLEAdvertisedDevice* pAdvertisedDevice) = 0;
  virtual bool init(NimBLEClient* pClient) = 0;
  virtual bool deinit() = 0;

  std::atomic_bool _pendingDeregistration;
  std::atomic<const NimBLEAddress*> _address;
  NimBLEClient* _pClient;

  ConnectionState _connectionState;
  NimBLEAddress _lastAddress;
  blegc::BLEDeviceInfo _deviceInfo;
};

template <typename T>
class BLEBaseController : public BLEAbstractController {
 public:
  explicit BLEBaseController()
      : _onConnecting([](T&) {}), _onConnectionFailed([](T&) {}), _onConnected([](T&) {}), _onDisconnected([](T&) {}) {}

  void onConnecting(const std::function<void(T&)>& callback) { _onConnecting = callback; }

  void onConnectionFailed(const std::function<void(T&)>& callback) { _onConnectionFailed = callback; }

  /**
   * @brief Sets the callback to be invoked when the controller connects.
   * @param callback Reference to a callback function.
   */
  void onConnected(const std::function<void(T&)>& callback) { _onConnected = callback; }

  /**
   * @brief Sets the callback to be invoked when the controller disconnects.
   * @param callback Reference to the callback function.
   */
  void onDisconnected(const std::function<void(T&)>& callback) { _onDisconnected = callback; }

 protected:
  void callOnConnecting() override { _onConnecting(*static_cast<T*>(this)); }
  void callOnConnectionFailed() override { _onConnectionFailed(*static_cast<T*>(this)); }
  void callOnConnected() override { _onConnected(*static_cast<T*>(this)); }
  void callOnDisconnected() override { _onDisconnected(*static_cast<T*>(this)); }

 private:
  std::function<void(T&)> _onConnecting;
  std::function<void(T&)> _onConnectionFailed;
  std::function<void(T&)> _onConnected;
  std::function<void(T&)> _onDisconnected;
};
