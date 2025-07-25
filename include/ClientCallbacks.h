#pragma once

#include<NimBLEDevice.h>

class ClientCallbacks : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient* pClient) override;
  void onConnectFail(NimBLEClient* pClient, int reason) override;
  void onAuthenticationComplete(NimBLEConnInfo& connInfo) override;
  void onDisconnect(NimBLEClient* pClient, int reason) override;
};
