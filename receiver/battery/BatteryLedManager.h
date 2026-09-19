#pragma once

#include <Arduino.h>
#include "../config/BatteryLedConfig.h"
#include "INA219Driver.h"

// =====================================================
//                 LED PATTERN STATES
// =====================================================
enum BatteryLedMode {
  LED_OFF,
  LED_SOLID,
  LED_NORMAL_BLINK,
  LED_FAST_BLINK
};

// =====================================================
//               BATTERY LED MANAGER
// =====================================================
class BatteryLedManager {
public:
  BatteryLedManager();

  void begin();
  void update();

  float getVoltage() const;
  float getCurrent_mA() const;
  float getBatteryPercent() const;
  bool isCharging() const;

  // Calculation utilities
  static float calculatePercentage(float voltage);
  static bool isChargingCurrent(float current_mA);

private:
  INA219Driver _ina219;

  float _voltage;
  float _current_mA;
  float _batteryPercent;
  bool _charging;

  unsigned long _lastSensorRead;

  void updateSensors();
  void updateLeds();
  void driveLed(uint8_t pin, BatteryLedMode mode, bool normalBlinkOn, bool fastBlinkOn);
};

extern BatteryLedManager batteryLedManager;
