#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "BoardConfig.h"
#include "BaseConfigManager.h"

// =====================================================
//             BATTERY & INA219 HARDWARE CONFIG
// =====================================================

// INA219 I2C Hardware Settings (Fixed Hardware)
#define INA219_I2C_ADDRESS 0x40
#define INA219_I2C_SDA 21
#define INA219_I2C_SCL 22

// Battery Hardware Specs (BAK N18650CR-35E)
#define BATTERY_NOMINAL_VOLTAGE 3.6f
#define BATTERY_FULL_VOLTAGE 4.2f
#define BATTERY_CAPACITY_MAH 3500

// 5-LED GPIO Pin Mapping (Fixed Hardware, Active HIGH)
#define BATTERY_LED_1_PIN 13
#define BATTERY_LED_2_PIN 14
#define BATTERY_LED_3_PIN 25
#define BATTERY_LED_4_PIN 26
#define BATTERY_LED_5_PIN 32

// =====================================================
//             CONFIGURABLE BATTERY SETTINGS
// =====================================================

#define MAX_LED_THRESHOLDS 4
#define MAX_VOLTAGE_TABLE_POINTS 16

struct BatteryLedThreshold {
  float minPercent;
  uint8_t ledCount;
};

struct VoltagePercentPoint {
  float voltage;
  float percent;
};

struct BatterySettings {
  uint16_t schemaVersion;

  // Current thresholds (mA)
  float currentChargingThresholdMa;   // default -50.0f
  float currentDischargingThresholdMa;// default 50.0f

  // Percentage thresholds
  float batteryLedLowThreshold;       // default 10.0f
  float batteryChargingFullThreshold; // default 90.0f
  float batteryLedHysteresisPercent;  // default 2.0f
  float batteryVoltageEmaAlpha;       // default 0.25f

  // Intervals (ms)
  uint32_t batteryPollIntervalMs;     // default 200
  uint32_t batteryLedBlinkIntervalMs; // default 500

  // Tables
  uint8_t ledThresholdCount;
  BatteryLedThreshold ledThresholds[MAX_LED_THRESHOLDS];

  uint8_t voltageTableCount;
  VoltagePercentPoint voltageTable[MAX_VOLTAGE_TABLE_POINTS];
};

class BatteryConfigManager : public BaseConfigManager<BatteryConfigManager, BatterySettings> {
public:
  static const uint16_t CURRENT_SCHEMA_VERSION = 1;
  static const char* getNvsNamespace() { return "cfg_battery"; }
  static const char* getTag() { return "BatteryConfig"; }

  BatteryConfigManager();

  void loadDefaults();
  bool validate(const BatterySettings& settings, String& err);
};

extern BatteryConfigManager batteryConfig;
