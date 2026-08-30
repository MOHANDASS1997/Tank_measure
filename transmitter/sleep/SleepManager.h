#pragma once

#include <Arduino.h>
#include <esp_sleep.h>
#include "../config/BoardConfig.h"
#include "../config/TransmitterConfig.h"

// =====================================================
//                   SLEEP MANAGER
// =====================================================

class SleepManager {
public:
  SleepManager();

  void begin();
  unsigned long getNextSequenceNumber();
  unsigned long getBootCount() const;
  void goToDeepSleep(uint32_t seconds = DEEP_SLEEP_SECONDS);
};

extern SleepManager sleepManager;
