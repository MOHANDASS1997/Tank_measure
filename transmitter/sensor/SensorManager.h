#pragma once

#include <Arduino.h>
#include "../config/BoardConfig.h"
#include "../config/SensorConfig.h"

// =====================================================
//                   SENSOR MANAGER
// =====================================================

class SensorManager {
public:
  SensorManager();

  void begin();
  float measureSingleDistanceCm();
  float measureFilteredDistanceCm();

private:
  void triggerPulse();
};

extern SensorManager sensorManager;
