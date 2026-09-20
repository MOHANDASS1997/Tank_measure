#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "BaseConfigManager.h"

// =====================================================
//                  TANK CONFIGURATION
// =====================================================

#define MAX_TANKS 4

struct TankConfig {
  char tankId[16];
  float totalLengthCm;
  float totalCapacityLitres;
  float fullDistanceCm;
  float emptyDistanceCm;
};

struct TankSettings {
  uint16_t schemaVersion;
  uint8_t count;
  TankConfig tanks[MAX_TANKS];
};

class TankConfigManager : public BaseConfigManager<TankConfigManager, TankSettings> {
public:
  static const uint16_t CURRENT_SCHEMA_VERSION = 1;
  static const char* getNvsNamespace() { return "cfg_tank"; }
  static const char* getTag() { return "TankConfig"; }

  TankConfigManager();

  void loadDefaults();
  bool validate(const TankSettings& settings, String& err);

  bool findTank(const char* tankId, TankConfig& result) const;
  uint8_t getCount() const { return _settings.count; }
  const TankConfig* getTanks() const { return _settings.tanks; }

  bool addTank(const TankConfig& tank);
  bool updateTank(uint8_t index, const TankConfig& tank);
  bool removeTank(uint8_t index);
};

extern TankConfigManager tankConfig;