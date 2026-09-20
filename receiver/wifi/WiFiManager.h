#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>

#include "../config/WiFiConfig.h"
#include "../config/SystemConfig.h"

// Wi-Fi Consumer Bitmask
enum WiFiConsumer : uint8_t {
  CONSUMER_NONE        = 0x00,
  CONSUMER_TIMESTAMP   = 0x01,
  CONSUMER_CONFIG_MODE = 0x02
};

// Wi-Fi State Machine
enum WiFiState : uint8_t {
  WIFI_STATE_OFF,
  WIFI_STATE_STA_CONNECTING,
  WIFI_STATE_STA_CONNECTED,
  WIFI_STATE_AP_ACTIVE
};

class WiFiManager {
public:
  WiFiManager();

  void begin();
  void update();

  // Consumer control
  void requestTimestampSync();
  void enterConfigMode();
  void exitConfigMode();
  void toggleConfigMode();

  // Queries
  bool isConfigModeActive() const { return (_consumers & CONSUMER_CONFIG_MODE) != 0; }
  bool isTimestampSyncActive() const { return (_consumers & CONSUMER_TIMESTAMP) != 0; }
  bool isConnected() const { return (_state == WIFI_STATE_STA_CONNECTED); }
  bool isAPActive() const { return (_state == WIFI_STATE_AP_ACTIVE); }
  bool hasConfiguredSSID() const { return strlen(wifiConfig.get().ssid) > 0; }

  String getIP() const;
  String getSSID() const;
  String getHostname() const { return "http://dasshome.local"; }
  unsigned long getConfigModeRemainingSeconds() const;

private:
  uint8_t _consumers;
  WiFiState _state;

  unsigned long _staConnectStartTime;
  unsigned long _configModeStartTime;
  unsigned long _timestampSyncStartTime;

  WebServer _server;
  DNSServer _dnsServer;
  bool _mdnsStarted;
  bool _serverRoutesConfigured;
  bool _serverRunning;

  void applyPowerState();
  void startSTAConnection();
  void startAP();
  void setupWebServer();
  void stopWebServer();

  // HTTP route handlers
  void handleRoot();
  void handleGetConfig();
  void handlePostConfig();
  void handleResetConfig();
  void handleExitConfig();
  void handleCaptivePortal();
};

extern WiFiManager wifiManager;
