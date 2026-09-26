#include "TransmitterConfig.h"

TransmitterConfigManager transmitterConfig;

TransmitterConfigManager::TransmitterConfigManager() {
  loadDefaults();
}

// =====================================================
//                 LOAD DEFAULTS
// =====================================================

static void applyTxOpDefaults(TransmitterConfig& tx) {
  tx.samplesPerWake     = 5;
  tx.samplingIntervalMs = 30;

  // Default 3-slot time-based schedule:
  //   06:00 – 08:00  → every 10 min (600 s)  morning monitoring
  //   08:00 – 18:00  → every 30 min (1800 s) daytime
  //   18:00 – 06:00  → every 60 min (3600 s) overnight (spans midnight)
  tx.scheduleSlotCount = 3;
  tx.scheduleSlots[0] = { 6, 8,  600  };
  tx.scheduleSlots[1] = { 8, 18, 1800 };
  tx.scheduleSlots[2] = { 18, 6, 3600 };
  for (int s = tx.scheduleSlotCount; s < MAX_WAKE_SLOTS; s++) {
    tx.scheduleSlots[s] = { 0, 0, 0 };
  }
}

void TransmitterConfigManager::loadDefaults() {
  _settings.schemaVersion = CURRENT_SCHEMA_VERSION;
  _settings.count = 1;

  _settings.transmitters[0].transmitterAddress = 3201;
  strncpy(_settings.transmitters[0].tankId, "tank_1",
          sizeof(_settings.transmitters[0].tankId) - 1);
  _settings.transmitters[0].tankId[sizeof(_settings.transmitters[0].tankId) - 1] = '\0';
  _settings.transmitters[0].sensorMinDistanceCm = 25.0f;
  _settings.transmitters[0].sensorMaxDistanceCm = 400.0f;
  _settings.transmitters[0].batteryFullVoltage   = 4.20f;
  _settings.transmitters[0].batteryEmptyVoltage  = 3.20f;
  applyTxOpDefaults(_settings.transmitters[0]);

  for (int i = 1; i < MAX_TRANSMITTERS; i++) {
    memset(&_settings.transmitters[i], 0, sizeof(TransmitterConfig));
    applyTxOpDefaults(_settings.transmitters[i]);
  }
}

// =====================================================
//                    VALIDATE
// =====================================================

bool TransmitterConfigManager::validate(const TransmitterSettings& s, String& err) {
  if (s.count == 0 || s.count > MAX_TRANSMITTERS) {
    err = "Transmitter count must be between 1 and " + String(MAX_TRANSMITTERS);
    return false;
  }

  for (uint8_t i = 0; i < s.count; i++) {
    const TransmitterConfig& tx = s.transmitters[i];

    if (tx.transmitterAddress <= 0) {
      err = "Invalid transmitter address at index " + String(i);
      return false;
    }
    if (strlen(tx.tankId) == 0) {
      err = "Tank ID for transmitter " + String(tx.transmitterAddress) + " cannot be empty";
      return false;
    }
    if (tx.sensorMinDistanceCm >= tx.sensorMaxDistanceCm) {
      err = "Sensor min distance must be less than max for transmitter " + String(tx.transmitterAddress);
      return false;
    }
    if (tx.batteryFullVoltage <= tx.batteryEmptyVoltage) {
      err = "Battery full voltage must exceed empty for transmitter " + String(tx.transmitterAddress);
      return false;
    }
    if (tx.samplesPerWake < 1 || tx.samplesPerWake > 20) {
      err = "samplesPerWake must be 1-20 for transmitter " + String(tx.transmitterAddress);
      return false;
    }
    if (tx.samplingIntervalMs < 10 || tx.samplingIntervalMs > 5000) {
      err = "samplingIntervalMs must be 10-5000 for transmitter " + String(tx.transmitterAddress);
      return false;
    }
    if (tx.scheduleSlotCount == 0 || tx.scheduleSlotCount > MAX_WAKE_SLOTS) {
      err = "scheduleSlotCount must be 1-" + String(MAX_WAKE_SLOTS) + " for transmitter " + String(tx.transmitterAddress);
      return false;
    }
    for (uint8_t s2 = 0; s2 < tx.scheduleSlotCount; s2++) {
      const WakeScheduleSlot& sl = tx.scheduleSlots[s2];
      if (sl.startHour > 23 || sl.endHour > 23) {
        err = "Schedule slot hours must be 0-23 for transmitter " + String(tx.transmitterAddress);
        return false;
      }
      if (sl.startHour == sl.endHour) {
        err = "Schedule slot startHour must differ from endHour for transmitter " + String(tx.transmitterAddress);
        return false;
      }
      if (sl.wakeSec < 5 || sl.wakeSec > 86400) {
        err = "wakeSec must be 5-86400 for transmitter " + String(tx.transmitterAddress);
        return false;
      }
    }
  }

  return true;
}

// =====================================================
//             GETTERS / MUTATORS
// =====================================================

bool TransmitterConfigManager::findTransmitter(int transmitterAddress, TransmitterConfig& result) const {
  for (uint8_t i = 0; i < _settings.count; i++) {
    if (_settings.transmitters[i].transmitterAddress == transmitterAddress) {
      result = _settings.transmitters[i];
      return true;
    }
  }
  return false;
}

bool TransmitterConfigManager::addTransmitter(const TransmitterConfig& tx) {
  if (_settings.count >= MAX_TRANSMITTERS) return false;
  _settings.transmitters[_settings.count] = tx;
  _settings.count++;
  return true;
}

bool TransmitterConfigManager::updateTransmitter(uint8_t index, const TransmitterConfig& tx) {
  if (index >= _settings.count) return false;
  _settings.transmitters[index] = tx;
  return true;
}

bool TransmitterConfigManager::removeTransmitter(uint8_t index) {
  if (index >= _settings.count || _settings.count <= 1) return false;
  for (uint8_t i = index; i < _settings.count - 1; i++) {
    _settings.transmitters[i] = _settings.transmitters[i + 1];
  }
  _settings.count--;
  memset(&_settings.transmitters[_settings.count], 0, sizeof(TransmitterConfig));
  return true;
}

// =====================================================
//          EFFECTIVE WAKE DURATION LOOKUP
// =====================================================
//
// Resolves the correct wakeSec for the current local hour.
// Midnight-spanning slots (startHour > endHour) are matched
// when localHour >= startHour OR localHour < endHour.
// =====================================================

uint16_t TransmitterConfigManager::getEffectiveWakeSec(
  int transmitterAddress, uint8_t localHour
) const {
  TransmitterConfig cfg;
  if (!findTransmitter(transmitterAddress, cfg)) {
    Serial.printf("[TxConfig] getEffectiveWakeSec: transmitter %d not found, using 3600s\n",
                  transmitterAddress);
    return 3600;
  }

  for (uint8_t i = 0; i < cfg.scheduleSlotCount; i++) {
    const WakeScheduleSlot& sl = cfg.scheduleSlots[i];
    bool inSlot;
    if (sl.startHour <= sl.endHour) {
      // Normal daytime slot: e.g. 8 → 18
      inSlot = (localHour >= sl.startHour && localHour < sl.endHour);
    } else {
      // Midnight-spanning slot: e.g. 18 → 6
      inSlot = (localHour >= sl.startHour || localHour < sl.endHour);
    }
    if (inSlot) return sl.wakeSec;
  }

  // No slot matched — fall back to last slot or hard default
  if (cfg.scheduleSlotCount > 0) {
    return cfg.scheduleSlots[cfg.scheduleSlotCount - 1].wakeSec;
  }
  return 3600;
}

// =====================================================
//   ONE-TIME MIGRATION: original v1 struct → current
// =====================================================
//
// The original TransmitterConfig (before any op-config
// additions) had only 6 fields (sensor + battery).
// Detect by NVS blob size. If it matches the original
// struct size, copy the fields we know and fill new
// fields with defaults.  Any other unexpected size
// resets to factory defaults.
// =====================================================

struct TransmitterConfigV1 {
  int   transmitterAddress;
  char  tankId[16];
  float sensorMinDistanceCm;
  float sensorMaxDistanceCm;
  float batteryFullVoltage;
  float batteryEmptyVoltage;
};

struct TransmitterSettingsV1 {
  uint16_t schemaVersion;
  uint8_t  count;
  TransmitterConfigV1 transmitters[MAX_TRANSMITTERS];
};

bool TransmitterConfigManager::migrateFromV1IfNeeded() {
  Preferences prefs;
  prefs.begin(getNvsNamespace(), true);
  size_t storedLen = prefs.getBytesLength("settings");
  prefs.end();

  if (storedLen == sizeof(TransmitterSettings)) {
    // Already current size — no migration needed
    return false;
  }

  if (storedLen == sizeof(TransmitterSettingsV1)) {
    // Read original v1 blob
    TransmitterSettingsV1 old;
    prefs.begin(getNvsNamespace(), true);
    prefs.getBytes("settings", &old, sizeof(TransmitterSettingsV1));
    prefs.end();

    Serial.println("[TransmitterConfig] Migrating from v1 layout...");

    _settings.schemaVersion = CURRENT_SCHEMA_VERSION;
    _settings.count = (old.count >= 1 && old.count <= MAX_TRANSMITTERS) ? old.count : 1;

    for (uint8_t i = 0; i < MAX_TRANSMITTERS; i++) {
      memset(&_settings.transmitters[i], 0, sizeof(TransmitterConfig));
      if (i < old.count) {
        _settings.transmitters[i].transmitterAddress = old.transmitters[i].transmitterAddress;
        memcpy(_settings.transmitters[i].tankId, old.transmitters[i].tankId, 16);
        _settings.transmitters[i].sensorMinDistanceCm = old.transmitters[i].sensorMinDistanceCm;
        _settings.transmitters[i].sensorMaxDistanceCm = old.transmitters[i].sensorMaxDistanceCm;
        _settings.transmitters[i].batteryFullVoltage   = old.transmitters[i].batteryFullVoltage;
        _settings.transmitters[i].batteryEmptyVoltage  = old.transmitters[i].batteryEmptyVoltage;
      }
      applyTxOpDefaults(_settings.transmitters[i]);
    }

    bool ok = save();
    Serial.println(ok ? "[TransmitterConfig] Migration successful."
                      : "[TransmitterConfig] Migration save failed.");
    return true;
  }

  // Unknown blob size — reset to factory defaults
  Serial.printf("[TransmitterConfig] Unknown NVS size (%u). Resetting to defaults.\n",
                (unsigned)storedLen);
  loadDefaults();
  save();
  return true;
}
