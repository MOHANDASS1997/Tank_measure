#include "TankConfig.h"

TankConfigManager tankConfig;

TankConfigManager::TankConfigManager() { loadDefaults(); }

void TankConfigManager::loadDefaults() {
  _settings.schemaVersion = CURRENT_SCHEMA_VERSION;
  _settings.count = 1;

  // Default tank 1 (from original firmware source of truth)
  strncpy(_settings.tanks[0].tankId, "tank_1",
          sizeof(_settings.tanks[0].tankId) - 1);
  _settings.tanks[0].tankId[sizeof(_settings.tanks[0].tankId) - 1] = '\0';
  _settings.tanks[0].totalLengthCm = 105.0f;
  _settings.tanks[0].totalCapacityLitres = 750.0f;
  _settings.tanks[0].fullDistanceCm = 25.0f;
  _settings.tanks[0].emptyDistanceCm = 105.0f;

  for (int i = 1; i < MAX_TANKS; i++) {
    memset(&_settings.tanks[i], 0, sizeof(TankConfig));
  }
}

bool TankConfigManager::validate(const TankSettings &s, String &err) {
  if (s.count == 0 || s.count > MAX_TANKS) {
    err = "Tank count must be between 1 and " + String(MAX_TANKS);
    return false;
  }

  for (uint8_t i = 0; i < s.count; i++) {
    if (strlen(s.tanks[i].tankId) == 0) {
      err = "Tank ID at index " + String(i) + " cannot be empty";
      return false;
    }
    if (s.tanks[i].totalCapacityLitres <= 0.0f) {
      err = "Capacity for " + String(s.tanks[i].tankId) + " must be positive";
      return false;
    }
    if (s.tanks[i].fullDistanceCm >= s.tanks[i].emptyDistanceCm) {
      err = "Full distance (" + String(s.tanks[i].fullDistanceCm, 1) +
            " cm) must be less than empty distance (" +
            String(s.tanks[i].emptyDistanceCm, 1) + " cm) for " +
            String(s.tanks[i].tankId);
      return false;
    }
    if (s.tanks[i].emptyDistanceCm > s.tanks[i].totalLengthCm) {
      err = "Empty distance exceeds total tank length for " +
            String(s.tanks[i].tankId);
      return false;
    }
  }

  return true;
}

bool TankConfigManager::findTank(const char *tankId, TankConfig &result) const {
  if (!tankId)
    return false;
  for (uint8_t i = 0; i < _settings.count; i++) {
    if (strcmp(_settings.tanks[i].tankId, tankId) == 0) {
      result = _settings.tanks[i];
      return true;
    }
  }
  return false;
}

bool TankConfigManager::addTank(const TankConfig &tank) {
  if (_settings.count >= MAX_TANKS)
    return false;
  _settings.tanks[_settings.count] = tank;
  _settings.count++;
  return true;
}

bool TankConfigManager::updateTank(uint8_t index, const TankConfig &tank) {
  if (index >= _settings.count)
    return false;
  _settings.tanks[index] = tank;
  return true;
}

bool TankConfigManager::removeTank(uint8_t index) {
  if (index >= _settings.count || _settings.count <= 1)
    return false;
  for (uint8_t i = index; i < _settings.count - 1; i++) {
    _settings.tanks[i] = _settings.tanks[i + 1];
  }
  _settings.count--;
  memset(&_settings.tanks[_settings.count], 0, sizeof(TankConfig));
  return true;
}
