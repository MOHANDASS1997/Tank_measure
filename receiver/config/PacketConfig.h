#pragma once

// =====================================================
//                 PACKET CONFIGURATION
// =====================================================

struct PacketFieldConfig {
  bool enabled;
  bool required;
};

struct PacketConfig {
  bool requireHeader;
  bool requireVersion;

  PacketFieldConfig sequence;
  PacketFieldConfig distance;
  PacketFieldConfig batteryVoltage;
  PacketFieldConfig charging;
};

const PacketConfig packetConfig = {
  true,   // requireHeader
  true,   // requireVersion

  // enabled, required
  { true, true },   // sequence
  { true, true },   // distance
  { true, true },   // battery voltage
  { true, true }    // charging
};

const char* const PACKET_HEADER = "TS";
const int SUPPORTED_PACKET_VERSION = 1;