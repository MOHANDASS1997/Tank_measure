#pragma once

#include <Arduino.h>

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
};

extern BatteryManager batteryManager;
