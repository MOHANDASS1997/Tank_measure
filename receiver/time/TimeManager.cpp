#include "TimeManager.h"

// Instantiate global TimeManager
TimeManager timeManager;

TimeManager::TimeManager()
  : _configured(false) {
}

void TimeManager::begin(long gmtOffset, int daylightOffset) {

  Serial.println("Configuring SNTP time sync via Wi-Fi...");
  configTime(gmtOffset, daylightOffset, timeConfig.get().ntpServer1, timeConfig.get().ntpServer2);
  _configured = true;
}

bool TimeManager::isSynced() const {

  time_t now = time(nullptr);
  // Valid Unix timestamp is after year 2021 (1609459200)
  return (now > 1609459200);
}

uint32_t TimeManager::getEpoch() const {

  time_t now = time(nullptr);
  if (now > 1609459200) {
    return (uint32_t)now;
  }
  return 0;
}

String TimeManager::formatElapsed(uint32_t pastTimestamp, unsigned long pastMillis) const {

  uint32_t diffSec = 0;
  uint32_t currentEpoch = getEpoch();

  if (pastTimestamp > 0 && currentEpoch > 0 && currentEpoch >= pastTimestamp) {
    diffSec = currentEpoch - pastTimestamp;
  } else {
    diffSec = (millis() - pastMillis) / 1000;
  }

  // < 60 seconds
  if (diffSec < 60) {
    return String(diffSec) + "s ago";
  }

  // < 60 minutes
  if (diffSec < 3600) {
    int mins = diffSec / 60;
    if (mins <= 1) {
      return "1min ago";
    }
    return String(mins) + "mins ago";
  }

  // < 24 hours
  if (diffSec < 86400) {
    float hrs = diffSec / 3600.0f;
    float remainder = fmod(hrs, 1.0f);
    if (remainder < 0.08f || remainder > 0.92f) {
      int intHrs = (int)round(hrs);
      if (intHrs <= 1) {
        return "1hr ago";
      }
      return String(intHrs) + "hrs ago";
    } else {
      return String(hrs, 1) + "hrs ago";
    }
  }

  // >= 24 hours (Days)
  float days = diffSec / 86400.0f;
  float remainder = fmod(days, 1.0f);
  if (remainder < 0.08f || remainder > 0.92f) {
    int intDays = (int)round(days);
    if (intDays <= 1) {
      return "1 day ago";
    }
    return String(intDays) + " days ago";
  } else {
    return String(days, 1) + " days ago";
  }
}
