#include "TxOpConfig.h"

// Instantiate global TxOpConfig
TxOpConfig txOpConfig;

const char* TxOpConfig::NVS_NAMESPACE = "tx_op_cfg";
const char* TxOpConfig::NVS_KEY       = "settings";

TxOpConfig::TxOpConfig() {
  _applyDefaults();
}

void TxOpConfig::_applyDefaults() {
  _settings.wakeDurationSec    = DEFAULT_WAKE_DURATION_SEC;
  _settings.samplesPerWake     = DEFAULT_SAMPLES_PER_WAKE;
  _settings.samplingIntervalMs = DEFAULT_SAMPLING_INTERVAL_MS;
}

bool TxOpConfig::validate(const TxOpSettings& s, String& err) const {
  if (s.wakeDurationSec < 5 || s.wakeDurationSec > 3600) {
    err = "wakeDurationSec must be 5-3600 s";
    return false;
  }
  if (s.samplesPerWake < 1 || s.samplesPerWake > 20) {
    err = "samplesPerWake must be 1-20";
    return false;
  }
  if (s.samplingIntervalMs < 10 || s.samplingIntervalMs > 5000) {
    err = "samplingIntervalMs must be 10-5000 ms";
    return false;
  }
  return true;
}

void TxOpConfig::begin() {
  _prefs.begin(NVS_NAMESPACE, true); // read-only
  size_t len = _prefs.getBytesLength(NVS_KEY);
  _prefs.end();

  if (len != sizeof(TxOpSettings)) {
    Serial.printf("[TxOpConfig] NVS size mismatch (%u vs %u). Using defaults.\n",
                  (unsigned)len, (unsigned)sizeof(TxOpSettings));
    _applyDefaults();
    save();
    return;
  }

  TxOpSettings tmp;
  _prefs.begin(NVS_NAMESPACE, true);
  _prefs.getBytes(NVS_KEY, &tmp, sizeof(TxOpSettings));
  _prefs.end();

  String err;
  if (!validate(tmp, err)) {
    Serial.print("[TxOpConfig] Stored settings invalid: ");
    Serial.println(err);
    Serial.println("[TxOpConfig] Resetting to defaults.");
    _applyDefaults();
    save();
    return;
  }

  _settings = tmp;
  Serial.printf("[TxOpConfig] Loaded: wds=%u s, sps=%u, sim=%u ms\n",
                _settings.wakeDurationSec,
                _settings.samplesPerWake,
                _settings.samplingIntervalMs);
}

bool TxOpConfig::save() {
  String err;
  if (!validate(_settings, err)) {
    Serial.print("[TxOpConfig] Cannot save invalid settings: ");
    Serial.println(err);
    return false;
  }

  _prefs.begin(NVS_NAMESPACE, false); // read-write
  size_t written = _prefs.putBytes(NVS_KEY, &_settings, sizeof(TxOpSettings));
  _prefs.end();

  bool ok = (written == sizeof(TxOpSettings));
  if (ok) {
    Serial.printf("[TxOpConfig] Saved: wds=%u s, sps=%u, sim=%u ms\n",
                  _settings.wakeDurationSec,
                  _settings.samplesPerWake,
                  _settings.samplingIntervalMs);
  } else {
    Serial.println("[TxOpConfig] NVS write failed.");
  }
  return ok;
}
