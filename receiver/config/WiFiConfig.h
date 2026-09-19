#pragma once

#include <Arduino.h>
#include <Preferences.h>

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

class WiFiConfigManager {
public:
  static const uint16_t CURRENT_SCHEMA_VERSION = 1;

  WiFiConfigManager();

  void begin();
  void loadDefaults();
  bool load();
  bool save();
  bool validate(const WiFiSettings& settings, String& err);

  const WiFiSettings& get() const { return _settings; }
  void set(const WiFiSettings& settings) { _settings = settings; }

  void setCredentials(const char* ssid, const char* pass);

private:
  WiFiSettings _settings;
  Preferences _prefs;
};

extern WiFiConfigManager wifiConfig;
