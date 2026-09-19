#pragma once

// =====================================================
//                     BOARD CONFIG
// =====================================================

// OLED I2C Pins
#define OLED_SDA 21
#define OLED_SCL 22

// Button Pin
#define BUTTON_PIN 27

// LoRa UART Pins
#define LORA_RX 16
#define LORA_TX 17

// =====================================================
//                  MOCK DATA LAYER
// =====================================================
// Set to true to simulate telemetry data, false for real LoRa data
#define USE_MOCK_DATA false
#define MOCK_INITIAL_DELAY_MS                                                  \
  10000 // Initial delay before 1st mock trigger (10 seconds)
#define MOCK_DATA_INTERVAL_MS 60000 // Periodic interval (1 minute)

// =====================================================
//                     TEST MODE
// =====================================================
// Set to true to activate Test Mode with diagnostic screens.
// In Test Mode, only test screens are displayed and can be navigated
// using the page navigation button. Set to false for normal operation.
#define TEST_MODE false
// =====================================================
//             DISPLAY & UI TIMEOUT CONFIG
// =====================================================
// Inactivity timeout before OLED (and LEDs when discharging) turns OFF
// (default: 15s)
#define UI_TIMEOUT_MS 15000

// Duration of the non-blocking charging splash animation (in milliseconds)
#define CHARGING_ANIMATION_DURATION_MS 2500