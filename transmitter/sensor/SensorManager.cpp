#include "SensorManager.h"

// Instantiate global SensorManager
SensorManager sensorManager;

SensorManager::SensorManager() {
}

void SensorManager::begin() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
}

void SensorManager::triggerPulse() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
}

float SensorManager::measureSingleDistanceCm() {
  triggerPulse();

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, sensorConfig.echoTimeoutUs);

  if (duration == 0) {
    return -1.0f; // No echo / timeout
  }

  return (float)duration / sensorConfig.speedOfSoundDivisor;
}

float SensorManager::measureFilteredDistanceCm() {
  float readings[sensorConfig.samplesCount];
  int validCount = 0;

  for (int i = 0; i < sensorConfig.samplesCount; i++) {
    float dist = measureSingleDistanceCm();
    if (dist > 0.0f) {
      readings[validCount++] = dist;
    }
    if (i < sensorConfig.samplesCount - 1) {
      delay(sensorConfig.sampleIntervalMs);
    }
  }

  if (validCount == 0) {
    return -1.0f; // All readings timed out
  }

  // Sort readings to find median
  for (int i = 0; i < validCount - 1; i++) {
    for (int j = i + 1; j < validCount; j++) {
      if (readings[i] > readings[j]) {
        float temp = readings[i];
        readings[i] = readings[j];
        readings[j] = temp;
      }
    }
  }

  // Return median reading
  return readings[validCount / 2];
}
