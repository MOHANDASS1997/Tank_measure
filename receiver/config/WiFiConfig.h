#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "BaseConfigManager.h"

// =====================================================
//                     WIFI CONFIG
// =====================================================

// Firmware compile-time defaults
#define DEFAULT_WIFI_SSID ""
#define DEFAULT_WIFI_PASSWORD ""
#define WIFI_AP_SSID "DASSHOME-Setup"
#define WIFI_AP_PASSWORD "" // Leave empty for open network
#define DEFAULT_WIFI_CONNECT_TIMEOUT_MS 8000
#define WIFI_CONNECTED_DISPLAY_MS 3000

// Standard local mDNS hostname
#define MDNS_HOSTNAME "dasshome"

struct WiFiSettings {
  uint16_t schemaVersion;
  char ssid[33];
  char password[65];
  uint32_t connectTimeoutMs;
};

class WiFiConfigManager : public BaseConfigManager<WiFiConfigManager, WiFiSettings> {
public:
  static const uint16_t CURRENT_SCHEMA_VERSION = 1;
  static const char* getNvsNamespace() { return "cfg_wifi"; }
  static const char* getTag() { return "WiFiConfig"; }

  WiFiConfigManager();

  void begin();
  void loadDefaults();
  bool validate(const WiFiSettings& settings, String& err);

  void setCredentials(const char* ssid, const char* pass);
};

extern WiFiConfigManager wifiConfig;
