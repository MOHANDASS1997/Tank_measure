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
  LED_BLINK
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

  void setLedsEnabled(bool enabled);
  bool areLedsEnabled() const;

  // Calculation utilities
  static float calculatePercentage(float voltage);
  static bool isChargingCurrent(float current_mA);
  static uint8_t calculateLedCount(float batteryPercent);
  static uint8_t calculateLedCountWithHysteresis(uint8_t currentCount, float batteryPercent, float hysteresis);
  static bool updateLowBatteryWithHysteresis(bool currentLowBattery, float batteryPercent, float lowThreshold, float hysteresis);
  static bool updateChargingFullWithHysteresis(bool currentFull, float batteryPercent, float fullThreshold, float hysteresis);

private:
  INA219Driver _ina219;

  float _voltage;
  float _shunt_mV;
  float _load_voltage;
  float _current_mA;
  float _power_mW;
  float _batteryPercent;
  bool _charging;
  bool _ledsEnabled;
  bool _ledStates[5];

  // Hysteresis states
  uint8_t _activeLedCount;
  bool _isLowBattery;
  bool _isChargingFull;
  bool _firstSensorRead;

  unsigned long _lastSensorRead;

  void updateSensors();
  void updateLeds();
  bool driveLed(uint8_t pin, BatteryLedMode mode, bool blinkOn);
};

extern BatteryLedManager batteryLedManager;
