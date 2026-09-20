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
