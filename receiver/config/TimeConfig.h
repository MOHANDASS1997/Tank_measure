#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "BaseConfigManager.h"

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

class TimeConfigManager : public BaseConfigManager<TimeConfigManager, TimeSettings> {
public:
  static const uint16_t CURRENT_SCHEMA_VERSION = 1;
  static const char* getNvsNamespace() { return "cfg_time"; }
  static const char* getTag() { return "TimeConfig"; }

  TimeConfigManager();

  void loadDefaults();
  bool validate(const TimeSettings& settings, String& err);
};

extern TimeConfigManager timeConfig;
