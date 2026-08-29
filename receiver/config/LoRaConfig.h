#pragma once

// =====================================================
//                     LORA CONFIG
// =====================================================

struct LoRaConfig {
  unsigned long band;
  int networkId;
  int address;

  int spreadingFactor;
  int bandwidth;
  int codingRate;
  int preambleLength;

  unsigned long baudRate;
};

const LoRaConfig loraConfig = {
  867000000UL,  // Band
  18,           // Network ID
  0,            // Receiver address

  9,            // SF
  7,            // BW
  1,            // Coding rate
  12,           // Preamble

  115200        // UART baud
};

const unsigned long LORA_TIMEOUT_MS = 15000;