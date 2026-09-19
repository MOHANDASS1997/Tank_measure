#pragma once

#include <Arduino.h>

// =====================================================
//             BATTERY & INA219 CONFIGURATION
// =====================================================

// Test mode alias (configured via TEST_MODE in BoardConfig.h)
#ifndef INA219_TEST_MODE
#define INA219_TEST_MODE TEST_MODE
#endif

// INA219 I2C Hardware Settings
#define INA219_I2C_ADDRESS 0x40
#define INA219_I2C_SDA 21
#define INA219_I2C_SCL 22

// Battery Specs (BAK N18650CR-35E)
#define BATTERY_NOMINAL_VOLTAGE 3.6f
#define BATTERY_FULL_VOLTAGE 4.2f
#define BATTERY_CAPACITY_MAH 3500

// Current Detection Thresholds (in mA)
// current < -50 mA  -> CHARGING
// current > +50 mA  -> DISCHARGING
// -50 mA to +50 mA  -> NOT CHARGING
#define CURRENT_CHARGING_THRESHOLD_MA -50.0f
#define CURRENT_DISCHARGING_THRESHOLD_MA 50.0f

// 5-LED GPIO Pin Mapping (Active HIGH: HIGH = ON, LOW = OFF)
// LED1 - LED4 = Battery level indicators (SOC)
// LED5        = Charging indicator
#define BATTERY_LED_1_PIN 13
#define BATTERY_LED_2_PIN 14
#define BATTERY_LED_3_PIN 25
#define BATTERY_LED_4_PIN 26
#define BATTERY_LED_5_PIN 32

// Blink Timing Interval (in milliseconds: 500ms ON / 500ms OFF)
#define BATTERY_LED_BLINK_INTERVAL_MS 500

// Periodic Sensor Polling Interval (in milliseconds)
#define BATTERY_POLL_INTERVAL_MS 200

// =====================================================
//      CONFIGURABLE BATTERY PERCENTAGE TO LED COUNT
// =====================================================

// Critical Low Battery threshold (Below this: LED1 blinks when discharging)
#define BATTERY_LED_LOW_THRESHOLD 10.0f

// Charging Full Threshold (LED5: Blinks while charging <90%, Solid glow when
// >=90%)
#define BATTERY_CHARGING_FULL_THRESHOLD 90.0f

// Hysteresis deadband in % to prevent threshold chattering around borders
// (+/- 2.0%)
#define BATTERY_LED_HYSTERESIS_PERCENT 2.0f

// Exponential Moving Average filter alpha for voltage (0.0 to 1.0; smooths out
// transient load dips)
#define BATTERY_VOLTAGE_EMA_ALPHA 0.25f

// Configurable Battery Percentage to Active LED Count mapping (for LED1 - LED4)
struct BatteryLedThreshold {
  float minPercent;
  uint8_t ledCount;
};

// Evaluated top-to-bottom: first condition (batteryPercent > minPercent)
// determines active LED count
static const BatteryLedThreshold BATTERY_LED_THRESHOLDS[] = {
    {85.0f, 4}, // > 85% -> 4 LEDs ON (LED1, LED2, LED3, LED4)
    {60.0f, 3}, // > 60% -> 3 LEDs ON (LED1, LED2, LED3)
    {25.0f, 2}, // > 25% -> 2 LEDs ON (LED1, LED2)
    {0.0f, 1}   // <= 25% -> 1 LED ON  (LED1)
};

static const size_t BATTERY_LED_THRESHOLDS_COUNT =
    sizeof(BATTERY_LED_THRESHOLDS) / sizeof(BATTERY_LED_THRESHOLDS[0]);

// Voltage-to-Percentage Mapping Table Struct
struct VoltagePercentPoint {
  float voltage;
  float percent;
};

// Exact Voltage-to-Percentage Lookup Points
// 4.20V -> 100%
// 4.10V ->  90%
// 4.00V ->  80%
// 3.90V ->  70%
// 3.80V ->  60%
// 3.70V ->  50%
// 3.60V ->  40%
// 3.50V ->  30%
// 3.40V ->  20%
// 3.30V ->  10%
// 3.20V ->   5%
// 3.00V ->   0%
static const VoltagePercentPoint VOLTAGE_PERCENT_TABLE[] = {
    {4.20f, 100.0f}, {4.10f, 90.0f}, {4.00f, 80.0f}, {3.90f, 70.0f},
    {3.80f, 60.0f},  {3.70f, 50.0f}, {3.60f, 40.0f}, {3.50f, 30.0f},
    {3.40f, 20.0f},  {3.30f, 10.0f}, {3.20f, 5.0f},  {3.00f, 0.0f}};

static const size_t VOLTAGE_PERCENT_TABLE_SIZE =
    sizeof(VOLTAGE_PERCENT_TABLE) / sizeof(VOLTAGE_PERCENT_TABLE[0]);
