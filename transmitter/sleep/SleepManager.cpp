#include "SleepManager.h"

// Instantiate global SleepManager
SleepManager sleepManager;

// Variables stored in ESP32-C3 RTC Fast Memory (persist across deep sleep)
RTC_DATA_ATTR static unsigned long rtcBootCount = 0;
RTC_DATA_ATTR static unsigned long rtcPacketSequence = 0;

SleepManager::SleepManager() {
}

void SleepManager::begin() {
  rtcBootCount++;

  esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();

  Serial.println("==================================================");
  Serial.print("ESP32-C3 Boot Count: ");
  Serial.println(rtcBootCount);

  Serial.print("Wakeup Reason: ");
  switch (wakeupReason) {
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Timer (Deep Sleep wakeup)");
      break;
    default:
      Serial.println("Power-on / Reset");
      break;
  }
  Serial.println("==================================================");
}

unsigned long SleepManager::getNextSequenceNumber() {
  rtcPacketSequence++;

  // Auto-rollover when reaching the configured maximum limit to prevent memory/overflow issues
  if (rtcPacketSequence >= MAX_SEQUENCE_NUMBER) {
    Serial.println("Sequence reached MAX_SEQUENCE_NUMBER. Resetting sequence to 1.");
    rtcPacketSequence = 1;
  }

  return rtcPacketSequence;
}

unsigned long SleepManager::getBootCount() const {
  return rtcBootCount;
}

void SleepManager::goToDeepSleep(uint32_t seconds) {
  Serial.print("Entering deep sleep for ");
  Serial.print(seconds);
  Serial.println(" seconds...");
  Serial.flush();

  // Configure timer wakeup in microseconds
  esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);

  // Enter deep sleep
  esp_deep_sleep_start();
}
