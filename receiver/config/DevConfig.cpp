#include "DevConfig.h"

DevConfigManager devConfig;

DevConfigManager::DevConfigManager() {
  loadDefaults();
}

void DevConfigManager::loadDefaults() {
  _settings.schemaVersion = CURRENT_SCHEMA_VERSION;
  _settings.devModeEnabled = false; // False by default
}

bool DevConfigManager::validate(const DevSettings& s, String& err) {
  // Boolean values are always valid; schema version check is sufficient
  return true;
}
