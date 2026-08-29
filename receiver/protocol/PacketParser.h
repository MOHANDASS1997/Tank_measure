#pragma once

#include <Arduino.h>
#include "../config/PacketConfig.h"
#include "../models/Telemetry.h"

// =====================================================
//              APPLICATION PACKET PARSER
// =====================================================

bool parseApplicationPacket(
  const String& payload,
  int sourceAddress,
  ParsedPacket& result
);
