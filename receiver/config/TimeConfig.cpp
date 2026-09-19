#include "TimeConfig.h"

TimeConfigManager timeConfig;

TimeConfigManager::TimeConfigManager() {
  loadDefaults();
}

void TimeConfigManager::loadDefaults() {
  _settings.schemaVersion = CURRENT_SCHEMA_VERSION;
  strncpy(_settings.ntpServer1, DEFAULT_NTP_SERVER_1, sizeof(_settings.ntpServer1) - 1);
  _settings.ntpServer1[sizeof(_settings.ntpServer1) - 1] = '\0';
  strncpy(_settings.ntpServer2, DEFAULT_NTP_SERVER_2, sizeof(_settings.ntpServer2) - 1);
  _settings.ntpServer2[sizeof(_settings.ntpServer2) - 1] = '\0';
  _settings.gmtOffsetSec = DEFAULT_GMT_OFFSET_SEC;
  _settings.daylightOffsetSec = DEFAULT_DAYLIGHT_OFFSET_SEC;
}

void TimeConfigManager::begin() {
  if (!load()) {
    Serial.println("[TimeConfig] No valid stored configuration found. Writing defaults.");
    loadDefaults();
    save();
  } else {
    Serial.println("[TimeConfig] Loaded persistent configuration successfully.");
  }
}

bool TimeConfigManager::load() {
  _prefs.begin("cfg_time", true);
  size_t len = _prefs.getBytesLength("settings");
  if (len != sizeof(TimeSettings)) {
    _prefs.end();
    return false;
  }

  TimeSettings temp;
  _prefs.getBytes("settings", &temp, sizeof(TimeSettings));
  _prefs.end();

  if (temp.schemaVersion != CURRENT_SCHEMA_VERSION) {
    Serial.println("[TimeConfig] Schema version mismatch; loading defaults.");
    return false;
  }

  String err;
  if (!validate(temp, err)) {
    Serial.print("[TimeConfig] Validation failed: ");
    Serial.println(err);
    return false;
  }

  _settings = temp;
  return true;
}

bool TimeConfigManager::save() {
  String err;
  if (!validate(_settings, err)) {
    Serial.print("[TimeConfig] Cannot save invalid settings: ");
    Serial.println(err);
    return false;
  }

  _prefs.begin("cfg_time", false);
  size_t written = _prefs.putBytes("settings", &_settings, sizeof(TimeSettings));
  _prefs.end();

  return (written == sizeof(TimeSettings));
}

bool TimeConfigManager::validate(const TimeSettings& s, String& err) {
  if (strlen(s.ntpServer1) == 0) {
    err = "Primary NTP Server cannot be empty";
    return false;
  }
  if (s.gmtOffsetSec < -43200 || s.gmtOffsetSec > 50400) {
    err = "GMT offset must be between -12h and +14h in seconds";
    return false;
  }
  return true;
}
