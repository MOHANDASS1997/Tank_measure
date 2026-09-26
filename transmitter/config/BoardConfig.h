#pragma once

// =====================================================
//             TRANSMITTER BOARD CONFIGURATION
//                  (ESP32-C3 Supermini)
// =====================================================

// Ultrasonic Sensor Pins
#define TRIG_PIN 4
#define ECHO_PIN 3

// LoRa RYLR998 UART Pins
#define LORA_RX_PIN 6 // ESP32-C3 receives from RYLR998 TXD
#define LORA_TX_PIN 7 // ESP32-C3 transmits to RYLR998 RXD

// INA219 Battery Monitor I2C Pins
#define INA219_SDA_PIN 0 // ESP32-C3 GPIO0 -> INA219 SDA
#define INA219_SCL_PIN 1 // ESP32-C3 GPIO1 -> INA219 SCL
#define INA219_I2C_ADDRESS 0x40

// Charging-state threshold: battery current < -10 mA means charging.
// Negative sign matches the INA219Driver convention (negative = charging).
// A magnitude of 10 mA avoids false triggers from measurement noise.
#define CHARGING_CURRENT_THRESHOLD_MA -10.0f

// =====================================================
//                 OPERATION MODE
// =====================================================
// Set to true for ultra-low power deep sleep (production)
// Set to false for standard delay loop (testing / continuous serial debugging)
#define ENABLE_DEEP_SLEEP true

// Transmission & Sleep Intervals (in seconds)
#define TRANSMIT_INTERVAL_SECONDS                                              \
  30 // Delay interval when ENABLE_DEEP_SLEEP is false (1 minute)
#define DEEP_SLEEP_SECONDS                                                     \
  60 // Deep sleep duration when ENABLE_DEEP_SLEEP is true (1 minute)
