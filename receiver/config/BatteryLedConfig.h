#pragma once

#include <Arduino.h>

// =====================================================
//             BATTERY & INA219 CONFIGURATION
// =====================================================

// Test mode alias (configured via TEST_MODE in BoardConfig.h)
#ifndef INA219_TEST_MODE
#define INA219_TEST_MODE                 TEST_MODE
#endif

// INA219 I2C Hardware Settings
#define INA219_I2C_ADDRESS               0x40
#define INA219_I2C_SDA                   21
#define INA219_I2C_SCL                   22

// Battery Specs (BAK N18650CR-35E)
#define BATTERY_NOMINAL_VOLTAGE          3.6f
#define BATTERY_FULL_VOLTAGE             4.2f
#define BATTERY_CAPACITY_MAH             3500

// Current Detection Thresholds (in mA)
// current < -50 mA  -> CHARGING
// current > +50 mA  -> DISCHARGING
// -50 mA to +50 mA  -> NOT CHARGING
#define CURRENT_CHARGING_THRESHOLD_MA    -50.0f
#define CURRENT_DISCHARGING_THRESHOLD_MA  50.0f

// 5-LED GPIO Pin Mapping (Active HIGH: HIGH = ON, LOW = OFF)
#define BATTERY_LED_1_PIN                13  // Band: 0 - <20%
#define BATTERY_LED_2_PIN                14  // Band: 20 - <40%
#define BATTERY_LED_3_PIN                25  // Band: 40 - <60%
#define BATTERY_LED_4_PIN                26  // Band: 60 - <80%
#define BATTERY_LED_5_PIN                32  // Band: 80 - 100%

// Blink Timing Intervals (in milliseconds)
#define NORMAL_BLINK_INTERVAL_MS         500
#define FAST_BLINK_INTERVAL_MS           200

// Periodic Sensor Polling Interval (in milliseconds)
#define BATTERY_POLL_INTERVAL_MS         200

// Voltage-to-Percentage Mapping Table Struct
struct VoltagePercentPoint {
  float voltage;
  float percent;
};

// Exact Voltage-to-Percentage Lookup Points
// 4.20V -> 100%
// 4.10V ->  90%
// 4.00V ->  80%
// 3.90V ->  65%
// 3.80V ->  50%
// 3.70V ->  35%
// 3.60V ->  20%
// 3.50V ->  10%
// 3.40V ->   5%
// 3.30V ->   0%
static const VoltagePercentPoint VOLTAGE_PERCENT_TABLE[] = {
  { 4.20f, 100.0f },
  { 4.10f,  90.0f },
  { 4.00f,  80.0f },
  { 3.90f,  65.0f },
  { 3.80f,  50.0f },
  { 3.70f,  35.0f },
  { 3.60f,  20.0f },
  { 3.50f,  10.0f },
  { 3.40f,   5.0f },
  { 3.30f,   0.0f }
};

static const size_t VOLTAGE_PERCENT_TABLE_SIZE =
  sizeof(VOLTAGE_PERCENT_TABLE) / sizeof(VOLTAGE_PERCENT_TABLE[0]);
