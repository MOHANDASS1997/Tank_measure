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

// =====================================================
//                 OPERATION MODE
// =====================================================
// Set to true for ultra-low power deep sleep (production)
// Set to false for standard delay loop (testing / continuous serial debugging)
#define ENABLE_DEEP_SLEEP false

// Transmission & Sleep Intervals (in seconds)
#define TRANSMIT_INTERVAL_SECONDS                                              \
  300 // Delay interval when ENABLE_DEEP_SLEEP is false (1 minute)
#define DEEP_SLEEP_SECONDS                                                     \
  60 // Deep sleep duration when ENABLE_DEEP_SLEEP is true (1 minute)
