#pragma once

#include <Arduino.h>
#include <time.h>
#include "../config/TimeConfig.h"

// =====================================================
//                    TIME MANAGER
// =====================================================

class TimeManager {
public:
  TimeManager();

  void begin(long gmtOffset = DEFAULT_GMT_OFFSET_SEC, int daylightOffset = DEFAULT_DAYLIGHT_OFFSET_SEC);
  bool isSynced() const;
  uint32_t getEpoch() const;
  String formatElapsed(uint32_t pastTimestamp, unsigned long pastMillis) const;

private:
  bool _configured;
};

extern TimeManager timeManager;
