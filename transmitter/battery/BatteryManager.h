#pragma once

#include <Arduino.h>
#include "../config/BoardConfig.h"
#include "INA219Driver.h"

// =====================================================
//                  BATTERY MANAGER
// =====================================================

class BatteryManager {
public:
  BatteryManager();

  void begin();
  void readBattery(float &voltage, bool &isCharging);
  void setHardwareMode(bool enabled);

private:
  bool _useHardwareADC;
  INA219Driver _ina219;
  bool _ina219Ok;
};

extern BatteryManager batteryManager;
