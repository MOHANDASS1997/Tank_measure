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
  float getShuntVoltage_mV() const;
  float getLoadVoltage_V() const;
  float getCurrent_mA() const;
  float getPower_mW() const;
  float getBatteryPercent() const;
  bool isCharging() const;
  const char* getChargingStateStr() const;
  bool isSensorConnected() const;
  bool getLedPinState(uint8_t index) const;

  // Calculation utilities
  static float calculatePercentage(float voltage);
  static bool isChargingCurrent(float current_mA);

private:
  INA219Driver _ina219;

  float _voltage;
  float _shunt_mV;
  float _load_voltage;
  float _current_mA;
  float _power_mW;
  float _batteryPercent;
  bool _charging;
  bool _ledStates[5];

  unsigned long _lastSensorRead;

  void updateSensors();
  void updateLeds();
  bool driveLed(uint8_t pin, BatteryLedMode mode, bool normalBlinkOn, bool fastBlinkOn);
};

extern BatteryLedManager batteryLedManager;
