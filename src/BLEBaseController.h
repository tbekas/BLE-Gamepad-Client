#pragma once

#include <functional>
#include "BLEAbstractController.h"

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
