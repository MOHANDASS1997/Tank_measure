#pragma once

#include <Arduino.h>
#include <Preferences.h>

// =====================================================
//                     TIME CONFIG
// =====================================================

#define DEFAULT_NTP_SERVER_1 "pool.ntp.org"
#define DEFAULT_NTP_SERVER_2 "time.google.com"
#define DEFAULT_GMT_OFFSET_SEC 19800
#define DEFAULT_DAYLIGHT_OFFSET_SEC 0

struct TimeSettings {
  uint16_t schemaVersion;
  char ntpServer1[48];
  char ntpServer2[48];
  long gmtOffsetSec;
  int daylightOffsetSec;
};

class TimeConfigManager {
public:
  static const uint16_t CURRENT_SCHEMA_VERSION = 1;

  TimeConfigManager();

  void begin();
  void loadDefaults();
  bool load();
  bool save();
  bool validate(const TimeSettings& settings, String& err);

  const TimeSettings& get() const { return _settings; }
  void set(const TimeSettings& settings) { _settings = settings; }

private:
  TimeSettings _settings;
  Preferences _prefs;
};

extern TimeConfigManager timeConfig;
