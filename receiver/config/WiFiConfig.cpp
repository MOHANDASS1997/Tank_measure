#include "WiFiConfig.h"

WiFiConfigManager wifiConfig;

WiFiConfigManager::WiFiConfigManager() {
  loadDefaults();
}

void WiFiConfigManager::loadDefaults() {
  _settings.schemaVersion = CURRENT_SCHEMA_VERSION;
  strncpy(_settings.ssid, DEFAULT_WIFI_SSID, sizeof(_settings.ssid) - 1);
  _settings.ssid[sizeof(_settings.ssid) - 1] = '\0';
  strncpy(_settings.password, DEFAULT_WIFI_PASSWORD, sizeof(_settings.password) - 1);
  _settings.password[sizeof(_settings.password) - 1] = '\0';
  _settings.connectTimeoutMs = DEFAULT_WIFI_CONNECT_TIMEOUT_MS;
}

void WiFiConfigManager::setCredentials(const char* ssid, const char* pass) {
  if (ssid) {
    strncpy(_settings.ssid, ssid, sizeof(_settings.ssid) - 1);
    _settings.ssid[sizeof(_settings.ssid) - 1] = '\0';
  }
  if (pass) {
    strncpy(_settings.password, pass, sizeof(_settings.password) - 1);
    _settings.password[sizeof(_settings.password) - 1] = '\0';
  }
}

void WiFiConfigManager::begin() {
  if (!load()) {
    // Check for legacy credentials from previous firmware release in "dasshome_wifi"
    Preferences legacyPrefs;
    legacyPrefs.begin("dasshome_wifi", true);
    String legacySsid = legacyPrefs.getString("ssid", "");
    String legacyPass = legacyPrefs.getString("pass", "");
    legacyPrefs.end();

    if (legacySsid.length() > 0) {
      Serial.println("[WiFiConfig] Migrating legacy credentials from 'dasshome_wifi'...");
      setCredentials(legacySsid.c_str(), legacyPass.c_str());
    } else {
      loadDefaults();
    }
    save();
  } else {
    Serial.println("[WiFiConfig] Loaded persistent configuration successfully.");
  }
}

bool WiFiConfigManager::load() {
  _prefs.begin("cfg_wifi", true);
  size_t len = _prefs.getBytesLength("settings");
  if (len != sizeof(WiFiSettings)) {
    _prefs.end();
    return false;
  }

  WiFiSettings temp;
  _prefs.getBytes("settings", &temp, sizeof(WiFiSettings));
  _prefs.end();

  if (temp.schemaVersion != CURRENT_SCHEMA_VERSION) {
    Serial.println("[WiFiConfig] Schema version mismatch; loading defaults.");
    return false;
  }

  String err;
  if (!validate(temp, err)) {
    Serial.print("[WiFiConfig] Validation failed: ");
    Serial.println(err);
    return false;
  }

  _settings = temp;
  return true;
}

bool WiFiConfigManager::save() {
  String err;
  if (!validate(_settings, err)) {
    Serial.print("[WiFiConfig] Cannot save invalid settings: ");
    Serial.println(err);
    return false;
  }

  _prefs.begin("cfg_wifi", false);
  size_t written = _prefs.putBytes("settings", &_settings, sizeof(WiFiSettings));
  _prefs.end();

  return (written == sizeof(WiFiSettings));
}

bool WiFiConfigManager::validate(const WiFiSettings& s, String& err) {
  if (s.connectTimeoutMs < 2000 || s.connectTimeoutMs > 30000) {
    err = "Wi-Fi connection timeout must be between 2s and 30s";
    return false;
  }
  return true;
}
