#pragma once

#include <Arduino.h>
#include "WiFiConfig.h"
#include "TankConfig.h"
#include "TransmitterConfig.h"
#include "BatteryLedConfig.h"
#include "SystemConfig.h"
#include "LoRaConfig.h"
#include "TimeConfig.h"

class ConfigJsonHelper {
public:
  static String serializeAll();
  static bool deserializeAndSave(const String& json, String& outError);
  static bool resetAllToDefaults(String& outError);
  static bool resetSection(const String& section, String& outError);
};
