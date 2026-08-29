#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

#include "../config/WiFiConfig.h"
#include "../config/BoardConfig.h"
#include "../display/DisplayManager.h"
#include "../input/ButtonManager.h"

// =====================================================
//                    WIFI MANAGER
// =====================================================

class WiFiManager {
public:
  WiFiManager();

  void begin();
  void runStartupFlow(DisplayManager& display, ButtonManager& button);

  bool isConnected() const;
  bool isAPActive() const;
  String getIP() const;
  String getSSID() const;

private:
  bool _connected;
  bool _isAP;
  String _ip;
  String _ssid;

  String _newSSID;
  String _newPassword;
  bool _pendingConnect;

  WebServer _server;
  DNSServer _dnsServer;
  Preferences _preferences;

  bool tryConnectSTA(const char* ssid, const char* password, unsigned long timeoutMs);
  void startAP();
  void setupWebServer();
  void handleRoot();
  void handleSave();
  void handleCaptivePortal();
  void saveCredentials(const String& ssid, const String& password);
  void loadCredentials(String& ssid, String& password);
};

extern WiFiManager wifiManager;
