#include "ConfigJsonHelper.h"

// Helper utilities for lightweight JSON parsing
static String extractSubBlock(const String& json, const String& key, char openChar, char closeChar) {
  int keyIdx = json.indexOf("\"" + key + "\"");
  if (keyIdx < 0) return "";
  int startIdx = json.indexOf(openChar, keyIdx);
  if (startIdx < 0) return "";

  int depth = 0;
  bool inQuote = false;
  for (int i = startIdx; i < (int)json.length(); i++) {
    char c = json[i];
    if (c == '"' && (i == 0 || json[i-1] != '\\')) {
      inQuote = !inQuote;
    } else if (!inQuote) {
      if (c == openChar) depth++;
      else if (c == closeChar) {
        depth--;
        if (depth == 0) {
          return json.substring(startIdx, i + 1);
        }
      }
    }
  }
  return "";
}

static String extractString(const String& json, const String& key, const String& defVal = "") {
  String pattern = "\"" + key + "\"";
  int keyIdx = json.indexOf(pattern);
  if (keyIdx < 0) return defVal;
  int colonIdx = json.indexOf(':', keyIdx + pattern.length());
  if (colonIdx < 0) return defVal;

  int valStart = colonIdx + 1;
  while (valStart < (int)json.length() && (json[valStart] == ' ' || json[valStart] == '\t' || json[valStart] == '\r' || json[valStart] == '\n')) {
    valStart++;
  }
  if (valStart >= (int)json.length()) return defVal;

  if (json[valStart] == '"') {
    int endQuote = json.indexOf('"', valStart + 1);
    if (endQuote > valStart) {
      return json.substring(valStart + 1, endQuote);
    }
  } else {
    int endVal = valStart;
    while (endVal < (int)json.length() && json[endVal] != ',' && json[endVal] != '}' && json[endVal] != ']' && json[endVal] != '\r' && json[endVal] != '\n') {
      endVal++;
    }
    return json.substring(valStart, endVal);
  }
  return defVal;
}

static float extractFloat(const String& json, const String& key, float defVal = 0.0f) {
  String str = extractString(json, key, "");
  if (str.length() == 0) return defVal;
  return str.toFloat();
}

static long extractLong(const String& json, const String& key, long defVal = 0) {
  String str = extractString(json, key, "");
  if (str.length() == 0) return defVal;
  return (long)str.toInt();
}

static void splitArrayObjects(const String& arrJson, std::vector<String>& outItems) {
  outItems.clear();
  int startIdx = arrJson.indexOf('[');
  int endIdx = arrJson.lastIndexOf(']');
  if (startIdx < 0 || endIdx <= startIdx) return;

  int depth = 0;
  bool inQuote = false;
  int objStart = -1;

  for (int i = startIdx + 1; i < endIdx; i++) {
    char c = arrJson[i];
    if (c == '"' && (i == 0 || arrJson[i-1] != '\\')) {
      inQuote = !inQuote;
    } else if (!inQuote) {
      if (c == '{') {
        if (depth == 0) objStart = i;
        depth++;
      } else if (c == '}') {
        depth--;
        if (depth == 0 && objStart >= 0) {
          outItems.push_back(arrJson.substring(objStart, i + 1));
          objStart = -1;
        }
      }
    }
  }
}

String ConfigJsonHelper::serializeAll() {
  const auto& wf = wifiConfig.get();
  const auto& tk = tankConfig.get();
  const auto& tx = transmitterConfig.get();
  const auto& bt = batteryConfig.get();
  const auto& sy = systemConfig.get();
  const auto& lr = loraConfigManager.get();
  const auto& tm = timeConfig.get();
  const auto& dv = devConfig.get();

  String out = "{\n";

  // Wi-Fi
  out += "  \"wifi\": {\n";
  out += "    \"ssid\": \"" + String(wf.ssid) + "\",\n";
  out += "    \"password\": \"" + String(wf.password) + "\",\n";
  out += "    \"connectTimeoutMs\": " + String(wf.connectTimeoutMs) + "\n";
  out += "  },\n";

  // Tanks
  out += "  \"tanks\": {\n";
  out += "    \"count\": " + String(tk.count) + ",\n";
  out += "    \"tanks\": [\n";
  for (uint8_t i = 0; i < tk.count; i++) {
    out += "      {\n";
    out += "        \"tankId\": \"" + String(tk.tanks[i].tankId) + "\",\n";
    out += "        \"totalLengthCm\": " + String(tk.tanks[i].totalLengthCm, 2) + ",\n";
    out += "        \"totalCapacityLitres\": " + String(tk.tanks[i].totalCapacityLitres, 2) + ",\n";
    out += "        \"fullDistanceCm\": " + String(tk.tanks[i].fullDistanceCm, 2) + ",\n";
    out += "        \"emptyDistanceCm\": " + String(tk.tanks[i].emptyDistanceCm, 2) + "\n";
    out += "      }" + String(i < tk.count - 1 ? "," : "") + "\n";
  }
  out += "    ]\n";
  out += "  },\n";

  // Transmitters
  out += "  \"transmitters\": {\n";
  out += "    \"count\": " + String(tx.count) + ",\n";
  out += "    \"transmitters\": [\n";
  for (uint8_t i = 0; i < tx.count; i++) {
    out += "      {\n";
    out += "        \"transmitterAddress\": " + String(tx.transmitters[i].transmitterAddress) + ",\n";
    out += "        \"tankId\": \"" + String(tx.transmitters[i].tankId) + "\",\n";
    out += "        \"sensorMinDistanceCm\": " + String(tx.transmitters[i].sensorMinDistanceCm, 2) + ",\n";
    out += "        \"sensorMaxDistanceCm\": " + String(tx.transmitters[i].sensorMaxDistanceCm, 2) + ",\n";
    out += "        \"batteryFullVoltage\": " + String(tx.transmitters[i].batteryFullVoltage, 2) + ",\n";
    out += "        \"batteryEmptyVoltage\": " + String(tx.transmitters[i].batteryEmptyVoltage, 2) + "\n";
    out += "      }" + String(i < tx.count - 1 ? "," : "") + "\n";
  }
  out += "    ]\n";
  out += "  },\n";

  // Battery
  out += "  \"battery\": {\n";
  out += "    \"currentChargingThresholdMa\": " + String(bt.currentChargingThresholdMa, 2) + ",\n";
  out += "    \"currentDischargingThresholdMa\": " + String(bt.currentDischargingThresholdMa, 2) + ",\n";
  out += "    \"batteryLedLowThreshold\": " + String(bt.batteryLedLowThreshold, 2) + ",\n";
  out += "    \"batteryChargingFullThreshold\": " + String(bt.batteryChargingFullThreshold, 2) + ",\n";
  out += "    \"batteryLedHysteresisPercent\": " + String(bt.batteryLedHysteresisPercent, 2) + ",\n";
  out += "    \"batteryVoltageEmaAlpha\": " + String(bt.batteryVoltageEmaAlpha, 3) + ",\n";
  out += "    \"batteryPollIntervalMs\": " + String(bt.batteryPollIntervalMs) + ",\n";
  out += "    \"batteryLedBlinkIntervalMs\": " + String(bt.batteryLedBlinkIntervalMs) + ",\n";
  out += "    \"voltageTableCount\": " + String(bt.voltageTableCount) + ",\n";
  out += "    \"voltageTable\": [\n";
  for (uint8_t i = 0; i < bt.voltageTableCount; i++) {
    out += "      {\"voltage\": " + String(bt.voltageTable[i].voltage, 2) + ", \"percent\": " + String(bt.voltageTable[i].percent, 1) + "}" + String(i < bt.voltageTableCount - 1 ? "," : "") + "\n";
  }
  out += "    ],\n";
  out += "    \"ledThresholdCount\": " + String(bt.ledThresholdCount) + ",\n";
  out += "    \"ledThresholds\": [\n";
  for (uint8_t i = 0; i < bt.ledThresholdCount; i++) {
    out += "      {\"minPercent\": " + String(bt.ledThresholds[i].minPercent, 1) + ", \"ledCount\": " + String(bt.ledThresholds[i].ledCount) + "}" + String(i < bt.ledThresholdCount - 1 ? "," : "") + "\n";
  }
  out += "    ]\n";
  out += "  },\n";

  // System
  out += "  \"system\": {\n";
  out += "    \"uiTimeoutMs\": " + String(sy.uiTimeoutMs) + ",\n";
  out += "    \"configTimeoutMs\": " + String(sy.configTimeoutMs) + ",\n";
  out += "    \"longPressDurationMs\": " + String(sy.longPressDurationMs) + ",\n";
  out += "    \"chargingAnimationDurationMs\": " + String(sy.chargingAnimationDurationMs) + ",\n";
  out += "    \"autoSleepEnabled\": " + String(sy.autoSleepEnabled ? "true" : "false") + "\n";
  out += "  },\n";

  // LoRa
  out += "  \"lora\": {\n";
  out += "    \"band\": " + String(lr.band) + ",\n";
  out += "    \"networkId\": " + String(lr.networkId) + ",\n";
  out += "    \"address\": " + String(lr.address) + ",\n";
  out += "    \"spreadingFactor\": " + String(lr.spreadingFactor) + ",\n";
  out += "    \"bandwidth\": " + String(lr.bandwidth) + ",\n";
  out += "    \"codingRate\": " + String(lr.codingRate) + ",\n";
  out += "    \"preambleLength\": " + String(lr.preambleLength) + ",\n";
  out += "    \"baudRate\": " + String(lr.baudRate) + "\n";
  out += "  },\n";

  // Time
  out += "  \"time\": {\n";
  out += "    \"ntpServer1\": \"" + String(tm.ntpServer1) + "\",\n";
  out += "    \"ntpServer2\": \"" + String(tm.ntpServer2) + "\",\n";
  out += "    \"gmtOffsetSec\": " + String(tm.gmtOffsetSec) + ",\n";
  out += "    \"daylightOffsetSec\": " + String(tm.daylightOffsetSec) + "\n";
  out += "  },\n";
  out += "  \"dev\": {\n";
  out += "    \"devModeEnabled\": " + String(dv.devModeEnabled ? "true" : "false") + "\n";
  out += "  }\n";

  out += "}";
  return out;
}

bool ConfigJsonHelper::deserializeAndSave(const String& json, String& outError) {
  // Staging copies
  WiFiSettings sWifi = wifiConfig.get();
  TankSettings sTank = tankConfig.get();
  TransmitterSettings sTx = transmitterConfig.get();
  BatterySettings sBattery = batteryConfig.get();
  SystemSettings sSystem = systemConfig.get();
  LoRaSettings sLoRa = loraConfigManager.get();
  TimeSettings sTime = timeConfig.get();
  DevSettings sDev = devConfig.get();

  // 1. Wi-Fi
  String wifiBlock = extractSubBlock(json, "wifi", '{', '}');
  if (wifiBlock.length() > 0) {
    String ssid = extractString(wifiBlock, "ssid", sWifi.ssid);
    String pass = extractString(wifiBlock, "password", sWifi.password);
    strncpy(sWifi.ssid, ssid.c_str(), sizeof(sWifi.ssid) - 1);
    sWifi.ssid[sizeof(sWifi.ssid) - 1] = '\0';
    strncpy(sWifi.password, pass.c_str(), sizeof(sWifi.password) - 1);
    sWifi.password[sizeof(sWifi.password) - 1] = '\0';
    sWifi.connectTimeoutMs = extractLong(wifiBlock, "connectTimeoutMs", sWifi.connectTimeoutMs);
  }

  // 2. Tanks
  String tanksArr = extractSubBlock(json, "tanks", '[', ']');
  if (tanksArr.length() > 0) {
    std::vector<String> items;
    splitArrayObjects(tanksArr, items);
    if (items.size() > 0 && items.size() <= MAX_TANKS) {
      sTank.count = items.size();
      for (size_t i = 0; i < items.size(); i++) {
        String tid = extractString(items[i], "tankId", "tank_1");
        strncpy(sTank.tanks[i].tankId, tid.c_str(), sizeof(sTank.tanks[i].tankId) - 1);
        sTank.tanks[i].tankId[sizeof(sTank.tanks[i].tankId) - 1] = '\0';
        sTank.tanks[i].totalLengthCm = extractFloat(items[i], "totalLengthCm", 180.0f);
        sTank.tanks[i].totalCapacityLitres = extractFloat(items[i], "totalCapacityLitres", 750.0f);
        sTank.tanks[i].fullDistanceCm = extractFloat(items[i], "fullDistanceCm", 15.0f);
        sTank.tanks[i].emptyDistanceCm = extractFloat(items[i], "emptyDistanceCm", 175.0f);
      }
    }
  }

  // 3. Transmitters
  String txArr = extractSubBlock(json, "transmitters", '[', ']');
  if (txArr.length() > 0) {
    std::vector<String> items;
    splitArrayObjects(txArr, items);
    if (items.size() > 0 && items.size() <= MAX_TRANSMITTERS) {
      sTx.count = items.size();
      for (size_t i = 0; i < items.size(); i++) {
        sTx.transmitters[i].transmitterAddress = extractLong(items[i], "transmitterAddress", 3201);
        String tid = extractString(items[i], "tankId", "tank_1");
        strncpy(sTx.transmitters[i].tankId, tid.c_str(), sizeof(sTx.transmitters[i].tankId) - 1);
        sTx.transmitters[i].tankId[sizeof(sTx.transmitters[i].tankId) - 1] = '\0';
        sTx.transmitters[i].sensorMinDistanceCm = extractFloat(items[i], "sensorMinDistanceCm", 25.0f);
        sTx.transmitters[i].sensorMaxDistanceCm = extractFloat(items[i], "sensorMaxDistanceCm", 400.0f);
        sTx.transmitters[i].batteryFullVoltage = extractFloat(items[i], "batteryFullVoltage", 4.20f);
        sTx.transmitters[i].batteryEmptyVoltage = extractFloat(items[i], "batteryEmptyVoltage", 3.20f);
      }
    }
  }

  // 4. Battery
  String batBlock = extractSubBlock(json, "battery", '{', '}');
  if (batBlock.length() > 0) {
    sBattery.currentChargingThresholdMa = extractFloat(batBlock, "currentChargingThresholdMa", sBattery.currentChargingThresholdMa);
    sBattery.currentDischargingThresholdMa = extractFloat(batBlock, "currentDischargingThresholdMa", sBattery.currentDischargingThresholdMa);
    sBattery.batteryLedLowThreshold = extractFloat(batBlock, "batteryLedLowThreshold", sBattery.batteryLedLowThreshold);
    sBattery.batteryChargingFullThreshold = extractFloat(batBlock, "batteryChargingFullThreshold", sBattery.batteryChargingFullThreshold);
    sBattery.batteryLedHysteresisPercent = extractFloat(batBlock, "batteryLedHysteresisPercent", sBattery.batteryLedHysteresisPercent);
    sBattery.batteryVoltageEmaAlpha = extractFloat(batBlock, "batteryVoltageEmaAlpha", sBattery.batteryVoltageEmaAlpha);
    sBattery.batteryPollIntervalMs = extractLong(batBlock, "batteryPollIntervalMs", sBattery.batteryPollIntervalMs);
    sBattery.batteryLedBlinkIntervalMs = extractLong(batBlock, "batteryLedBlinkIntervalMs", sBattery.batteryLedBlinkIntervalMs);

    // Voltage Table
    String voltArr = extractSubBlock(batBlock, "voltageTable", '[', ']');
    if (voltArr.length() > 0) {
      std::vector<String> vItems;
      splitArrayObjects(voltArr, vItems);
      if (vItems.size() >= 2 && vItems.size() <= MAX_VOLTAGE_TABLE_POINTS) {
        sBattery.voltageTableCount = vItems.size();
        for (size_t i = 0; i < vItems.size(); i++) {
          sBattery.voltageTable[i].voltage = extractFloat(vItems[i], "voltage", 0.0f);
          sBattery.voltageTable[i].percent = extractFloat(vItems[i], "percent", 0.0f);
        }
      }
    }

    // LED Thresholds
    String ledArr = extractSubBlock(batBlock, "ledThresholds", '[', ']');
    if (ledArr.length() > 0) {
      std::vector<String> lItems;
      splitArrayObjects(ledArr, lItems);
      if (lItems.size() > 0 && lItems.size() <= MAX_LED_THRESHOLDS) {
        sBattery.ledThresholdCount = lItems.size();
        for (size_t i = 0; i < lItems.size(); i++) {
          sBattery.ledThresholds[i].minPercent = extractFloat(lItems[i], "minPercent", 0.0f);
          sBattery.ledThresholds[i].ledCount = extractLong(lItems[i], "ledCount", 1);
        }
      }
    }
  }

  // 5. System
  String sysBlock = extractSubBlock(json, "system", '{', '}');
  if (sysBlock.length() > 0) {
    sSystem.uiTimeoutMs = extractLong(sysBlock, "uiTimeoutMs", sSystem.uiTimeoutMs);
    sSystem.configTimeoutMs = extractLong(sysBlock, "configTimeoutMs", sSystem.configTimeoutMs);
    sSystem.longPressDurationMs = extractLong(sysBlock, "longPressDurationMs", sSystem.longPressDurationMs);
    sSystem.chargingAnimationDurationMs = extractLong(sysBlock, "chargingAnimationDurationMs", sSystem.chargingAnimationDurationMs);
    String autoSleepStr = extractString(sysBlock, "autoSleepEnabled", sSystem.autoSleepEnabled ? "true" : "false");
    sSystem.autoSleepEnabled = (autoSleepStr == "true" || autoSleepStr == "1");
  }

  // 6. LoRa
  String loraBlock = extractSubBlock(json, "lora", '{', '}');
  if (loraBlock.length() > 0) {
    sLoRa.band = extractLong(loraBlock, "band", sLoRa.band);
    sLoRa.networkId = extractLong(loraBlock, "networkId", sLoRa.networkId);
    sLoRa.address = extractLong(loraBlock, "address", sLoRa.address);
    sLoRa.spreadingFactor = extractLong(loraBlock, "spreadingFactor", sLoRa.spreadingFactor);
    sLoRa.bandwidth = extractLong(loraBlock, "bandwidth", sLoRa.bandwidth);
    sLoRa.codingRate = extractLong(loraBlock, "codingRate", sLoRa.codingRate);
    sLoRa.preambleLength = extractLong(loraBlock, "preambleLength", sLoRa.preambleLength);
    sLoRa.baudRate = extractLong(loraBlock, "baudRate", sLoRa.baudRate);
  }

  // 7. Time
  String timeBlock = extractSubBlock(json, "time", '{', '}');
  if (timeBlock.length() > 0) {
    String n1 = extractString(timeBlock, "ntpServer1", sTime.ntpServer1);
    String n2 = extractString(timeBlock, "ntpServer2", sTime.ntpServer2);
    strncpy(sTime.ntpServer1, n1.c_str(), sizeof(sTime.ntpServer1) - 1);
    sTime.ntpServer1[sizeof(sTime.ntpServer1) - 1] = '\0';
    strncpy(sTime.ntpServer2, n2.c_str(), sizeof(sTime.ntpServer2) - 1);
    sTime.ntpServer2[sizeof(sTime.ntpServer2) - 1] = '\0';
    sTime.gmtOffsetSec = extractLong(timeBlock, "gmtOffsetSec", sTime.gmtOffsetSec);
    sTime.daylightOffsetSec = extractLong(timeBlock, "daylightOffsetSec", sTime.daylightOffsetSec);
  }

  // 8. Dev Mode
  String devBlock = extractSubBlock(json, "dev", '{', '}');
  if (devBlock.length() > 0) {
    String devStr = extractString(devBlock, "devModeEnabled", sDev.devModeEnabled ? "true" : "false");
    sDev.devModeEnabled = (devStr == "true" || devStr == "1");
  }

  // VALIDATION PHASE: Validate all 8 objects BEFORE saving any
  if (!wifiConfig.validate(sWifi, outError)) return false;
  if (!tankConfig.validate(sTank, outError)) return false;
  if (!transmitterConfig.validate(sTx, outError)) return false;
  if (!batteryConfig.validate(sBattery, outError)) return false;
  if (!systemConfig.validate(sSystem, outError)) return false;
  if (!loraConfigManager.validate(sLoRa, outError)) return false;
  if (!timeConfig.validate(sTime, outError)) return false;
  if (!devConfig.validate(sDev, outError)) return false;

  // COMMIT PHASE: Atomically apply and save to NVS
  wifiConfig.set(sWifi);
  wifiConfig.save();

  tankConfig.set(sTank);
  tankConfig.save();

  transmitterConfig.set(sTx);
  transmitterConfig.save();

  batteryConfig.set(sBattery);
  batteryConfig.save();

  systemConfig.set(sSystem);
  systemConfig.save();

  loraConfigManager.set(sLoRa);
  loraConfigManager.save();

  timeConfig.set(sTime);
  timeConfig.save();

  devConfig.set(sDev);
  devConfig.save();

  Serial.println("[Config] All 8 configuration objects updated and persisted successfully.");
  return true;
}

bool ConfigJsonHelper::resetAllToDefaults(String& outError) {
  wifiConfig.loadDefaults();
  wifiConfig.save();

  tankConfig.loadDefaults();
  tankConfig.save();

  transmitterConfig.loadDefaults();
  transmitterConfig.save();

  batteryConfig.loadDefaults();
  batteryConfig.save();

  systemConfig.loadDefaults();
  systemConfig.save();

  loraConfigManager.loadDefaults();
  loraConfigManager.save();

  timeConfig.loadDefaults();
  timeConfig.save();

  devConfig.loadDefaults();
  devConfig.save();

  Serial.println("[Config] All 8 configuration objects restored to factory defaults.");
  return true;
}

bool ConfigJsonHelper::resetSection(const String& section, String& outError) {
  if (section == "wifi") {
    wifiConfig.loadDefaults();
    wifiConfig.save();
    Serial.println("[Config] Wi-Fi settings reset to factory defaults.");
    return true;
  } else if (section == "tanks") {
    tankConfig.loadDefaults();
    tankConfig.save();
    Serial.println("[Config] Tank settings reset to factory defaults.");
    return true;
  } else if (section == "transmitters") {
    transmitterConfig.loadDefaults();
    transmitterConfig.save();
    Serial.println("[Config] Transmitter mappings reset to factory defaults.");
    return true;
  } else if (section == "battery") {
    batteryConfig.loadDefaults();
    batteryConfig.save();
    Serial.println("[Config] Battery & charge settings reset to factory defaults.");
    return true;
  } else if (section == "system") {
    systemConfig.loadDefaults();
    systemConfig.save();
    Serial.println("[Config] System & UI settings reset to factory defaults.");
    return true;
  } else if (section == "lora") {
    loraConfigManager.loadDefaults();
    loraConfigManager.save();
    Serial.println("[Config] LoRa settings reset to factory defaults.");
    return true;
  } else if (section == "time") {
    timeConfig.loadDefaults();
    timeConfig.save();
    Serial.println("[Config] Time settings reset to factory defaults.");
    return true;
  } else if (section == "dev") {
    devConfig.loadDefaults();
    devConfig.save();
    Serial.println("[Config] Dev Mode settings reset to factory defaults.");
    return true;
  } else if (section == "all" || section.length() == 0) {
    return resetAllToDefaults(outError);
  } else {
    outError = "Unknown configuration section: " + section;
    return false;
  }
}

