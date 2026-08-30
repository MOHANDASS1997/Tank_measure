#pragma once

// =====================================================
//               TRANSMITTER CONFIGURATION
// =====================================================

struct TransmitterLoRaConfig {
  int address;        // 3201
  int targetReceiver; // 3001
  int networkId;      // 18
  unsigned long band; // 867000000UL (867 MHz)

  int spreadingFactor; // 9
  int bandwidth;       // 7
  int codingRate;      // 1
  int preambleLength;  // 12
  int outputPower; // RF output power in dBm (0 to 22; 0 dBm is minimal power
                   // ~15-20mA)

  unsigned long baudRate; // 115200
};

const TransmitterLoRaConfig txConfig = {
    3201,        // Transmitter Address
    3001,        // Target Receiver Address
    18,          // Network ID
    867000000UL, // Frequency Band
    9,           // SF
    7,           // BW
    1,           // CR
    12,          // Preamble
    5,           // RF Output Power (0 dBm = ~1 mW, minimum current draw)
    115200       // Baud rate
};

// Protocol Parameters
const char *const TX_PACKET_HEADER = "TS";
const int TX_PACKET_VERSION = 1;

// Maximum sequence number before resetting to 1 (prevents unbounded character
// growth/overflow)
const unsigned long MAX_SEQUENCE_NUMBER = 1000000UL;
