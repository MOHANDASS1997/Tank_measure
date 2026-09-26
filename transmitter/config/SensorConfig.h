#pragma once

// =====================================================
//                 SENSOR CONFIGURATION
// =====================================================

struct SensorConfig {
  unsigned long echoTimeoutUs; // Ultrasonic echo timeout (30000us ~ 500cm)
  float speedOfSoundDivisor;   // 58.0 for cm conversion
  int samplesCount;            // Number of samples for median filtering
  int sampleIntervalMs;        // Delay between samples in ms
};

const SensorConfig sensorConfig = {
  30000,   // echoTimeoutUs
  58.0f,   // speedOfSoundDivisor
  5,       // samplesCount
  30       // sampleIntervalMs
};
