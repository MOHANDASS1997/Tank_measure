#include "WiFiManager.h"
#include "WebPortalHtml.h"
#include "../config/ConfigJsonHelper.h"
#include "../time/TimeManager.h"

WiFiManager wifiManager;

WiFiManager::WiFiManager()
  : _consumers(CONSUMER_NONE),
    _state(WIFI_STATE_OFF),
    _staConnectStartTime(0),
    _configModeStartTime(0),
    _timestampSyncStartTime(0),
    _server(80),
    _mdnsStarted(false) {
}

void WiFiManager::begin() {
  Serial.println("[WiFi] Initializing Wi-Fi Manager. Wi-Fi set to OFF by default.");
  WiFi.persistent(false);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  _state = WIFI_STATE_OFF;
  _consumers = CONSUMER_NONE;
}

void WiFiManager::applyPowerState() {
  if (_consumers == CONSUMER_NONE) {
    if (_state != WIFI_STATE_OFF) {
      Serial.println("[WiFi] No consumers remaining. Powering Wi-Fi OFF.");
      stopWebServer();
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      _state = WIFI_STATE_OFF;
    }
  } else {
    // At least one consumer needs Wi-Fi
    if (_state == WIFI_STATE_OFF) {
      startSTAConnection();
    }
  }
}

void WiFiManager::startSTAConnection() {
  const auto& wf = wifiConfig.get();
  if (strlen(wf.ssid) == 0) {
    Serial.println("[WiFi] No STA SSID configured.");
    if (isConfigModeActive()) {
      startAP();
    } else {
      Serial.println("[WiFi] Cannot connect STA without SSID. Releasing timestamp request.");
      _consumers &= ~CONSUMER_TIMESTAMP;
      applyPowerState();
    }
    return;
  }

  Serial.print("[WiFi] Connecting to SSID: ");
  Serial.println(wf.ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wf.ssid, wf.password);
  _state = WIFI_STATE_STA_CONNECTING;
  _staConnectStartTime = millis();
}

void WiFiManager::startAP() {
  Serial.println("[WiFi] Starting Fallback Access Point: " WIFI_AP_SSID);
  WiFi.mode(WIFI_AP_STA);
  if (strlen(WIFI_AP_PASSWORD) > 0) {
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
  } else {
    WiFi.softAP(WIFI_AP_SSID);
  }
  _state = WIFI_STATE_AP_ACTIVE;

  // Start captive DNS
  _dnsServer.start(53, "*", WiFi.softAPIP());
  setupWebServer();
}

void WiFiManager::setupWebServer() {
  // Endpoints
  _server.on("/", HTTP_GET, [this]() { handleRoot(); });
  _server.on("/api/config", HTTP_GET, [this]() { handleGetConfig(); });
  _server.on("/api/config", HTTP_POST, [this]() { handlePostConfig(); });
  _server.on("/api/reset", HTTP_POST, [this]() { handleResetConfig(); });
  _server.on("/api/exit", HTTP_POST, [this]() { handleExitConfig(); });

  // Browser icon handlers (prevent 302 redirect loops in AP mode)
  _server.on("/favicon.ico", HTTP_GET, [this]() { _server.send(204); });
  _server.on("/apple-touch-icon.png", HTTP_GET, [this]() { _server.send(204); });
  _server.on("/apple-touch-icon-precomposed.png", HTTP_GET, [this]() { _server.send(204); });

  // Captive portal redirects
  _server.on("/generate_204", [this]() { handleCaptivePortal(); });
  _server.on("/hotspot-detect.html", [this]() { handleCaptivePortal(); });
  _server.on("/canonical.html", [this]() { handleCaptivePortal(); });
  _server.on("/connecttest.txt", [this]() { handleCaptivePortal(); });
  _server.on("/ncsi.txt", [this]() { handleCaptivePortal(); });

  _server.onNotFound([this]() {
    if (_state == WIFI_STATE_AP_ACTIVE) {
      handleCaptivePortal();
    } else {
      _server.send(404, "text/plain", "Not Found");
    }
  });

  _server.begin();
  Serial.println("[WebUI] Web server started on port 80.");

  if (!_mdnsStarted) {
    if (MDNS.begin(MDNS_HOSTNAME)) {
      MDNS.addService("http", "tcp", 80);
      _mdnsStarted = true;
      Serial.println("[mDNS] Registered http://dasshome.local");
    } else {
      Serial.println("[mDNS] mDNS responder initialization failed.");
    }
  }
}

void WiFiManager::stopWebServer() {
  _server.stop();
  if (_state == WIFI_STATE_AP_ACTIVE) {
    _dnsServer.stop();
    WiFi.softAPdisconnect(true);
  }
  if (_mdnsStarted) {
    MDNS.end();
    _mdnsStarted = false;
  }
  Serial.println("[WebUI] Web server stopped.");
}

void WiFiManager::requestTimestampSync() {
  Serial.println("[WiFi] Requesting Wi-Fi for timestamp synchronization...");
  _consumers |= CONSUMER_TIMESTAMP;
  _timestampSyncStartTime = millis();

  // If already connected, trigger SNTP immediately
  if (_state == WIFI_STATE_STA_CONNECTED) {
    timeManager.begin(timeConfig.get().gmtOffsetSec, timeConfig.get().daylightOffsetSec);
  } else if (_state == WIFI_STATE_OFF) {
    startSTAConnection();
  }
}

void WiFiManager::enterConfigMode() {
  if (isConfigModeActive()) return;

  Serial.println("[WiFi] Entering Configuration Mode.");
  _consumers |= CONSUMER_CONFIG_MODE;
  _configModeStartTime = millis();

  if (_state == WIFI_STATE_STA_CONNECTED) {
    setupWebServer();
  } else if (_state == WIFI_STATE_OFF) {
    startSTAConnection();
  }
}

void WiFiManager::exitConfigMode() {
  if (!isConfigModeActive()) return;

  Serial.println("[WiFi] Exiting Configuration Mode.");
  stopWebServer();
  _consumers &= ~CONSUMER_CONFIG_MODE;
  applyPowerState();
}

void WiFiManager::toggleConfigMode() {
  if (isConfigModeActive()) {
    exitConfigMode();
  } else {
    enterConfigMode();
  }
}

void WiFiManager::update() {
  unsigned long now = millis();

  // 1. Check Configuration Mode Auto-Exit Timeout
  if (isConfigModeActive()) {
    unsigned long timeout = systemConfig.get().configTimeoutMs;
    if (now - _configModeStartTime >= timeout) {
      Serial.println("[WiFi] Configuration Mode 5-minute timeout reached. Automatically closing.");
      exitConfigMode();
      return;
    }

    // Handle web server clients and DNS requests
    _server.handleClient();
    if (_state == WIFI_STATE_AP_ACTIVE) {
      _dnsServer.processNextRequest();
    }
  }

  // 2. Wi-Fi STA Connection Progress & Bounded Timeout
  if (_state == WIFI_STATE_STA_CONNECTING) {
    if (WiFi.status() == WL_CONNECTED) {
      _state = WIFI_STATE_STA_CONNECTED;
      Serial.println("[WiFi] Connected to STA successfully!");
      Serial.print("[WiFi] IP Address: ");
      Serial.println(WiFi.localIP());

      // If configuration mode is active, ensure WebServer is running
      if (isConfigModeActive()) {
        setupWebServer();
      }

      // If timestamp sync requested, trigger SNTP sync
      if (isTimestampSyncActive()) {
        timeManager.begin(timeConfig.get().gmtOffsetSec, timeConfig.get().daylightOffsetSec);
      }
    } else {
      // Check bounded timeout
      unsigned long timeoutMs = wifiConfig.get().connectTimeoutMs;
      if (now - _staConnectStartTime >= timeoutMs) {
        Serial.println("[WiFi] STA connection timed out.");

        if (isConfigModeActive()) {
          Serial.println("[WiFi] Falling back to Access Point for Web UI.");
          startAP();
        } else {
          // Timestamp consumer timed out: fallback to relative time without blocking LoRa
          Serial.println("[WiFi] Proceeding with fallback timestamp. Releasing Wi-Fi requirement.");
          _consumers &= ~CONSUMER_TIMESTAMP;
          applyPowerState();
        }
      }
    }
  }

  // 3. Timestamp Sync Completion Check
  if (isTimestampSyncActive() && _state == WIFI_STATE_STA_CONNECTED) {
    // If SNTP is synced, or after bounded wait, release consumer
    if (timeManager.isSynced() || (now - _timestampSyncStartTime >= 4000)) {
      Serial.println("[WiFi] Timestamp operation complete. Releasing timestamp Wi-Fi consumer.");
      _consumers &= ~CONSUMER_TIMESTAMP;
      applyPowerState();
    }
  }
}

String WiFiManager::getIP() const {
  if (_state == WIFI_STATE_STA_CONNECTED) {
    return WiFi.localIP().toString();
  } else if (_state == WIFI_STATE_AP_ACTIVE) {
    return WiFi.softAPIP().toString();
  }
  return "0.0.0.0";
}

String WiFiManager::getSSID() const {
  if (_state == WIFI_STATE_STA_CONNECTED) {
    return String(wifiConfig.get().ssid);
  } else if (_state == WIFI_STATE_AP_ACTIVE) {
    return String(WIFI_AP_SSID);
  }
  return "";
}

unsigned long WiFiManager::getConfigModeRemainingSeconds() const {
  if (!isConfigModeActive()) return 0;
  unsigned long elapsed = millis() - _configModeStartTime;
  unsigned long total = systemConfig.get().configTimeoutMs;
  if (elapsed >= total) return 0;
  return (total - elapsed) / 1000;
}

// =====================================================
//                 HTTP HANDLERS
// =====================================================

void WiFiManager::handleRoot() {
  _configModeStartTime = millis(); // Reset inactivity timer on UI interactions
  _server.send_P(200, "text/html", WEB_PORTAL_HTML);
}

void WiFiManager::handleGetConfig() {
  _configModeStartTime = millis();
  String json = ConfigJsonHelper::serializeAll();
  _server.send(200, "application/json", json);
}

void WiFiManager::handlePostConfig() {
  _configModeStartTime = millis();
  if (!_server.hasArg("plain")) {
    _server.send(400, "application/json", "{\"success\":false,\"error\":\"Empty request body\"}");
    return;
  }

  String json = _server.arg("plain");
  String err;
  bool ok = ConfigJsonHelper::deserializeAndSave(json, err);

  if (ok) {
    _server.send(200, "application/json", "{\"success\":true,\"message\":\"Configuration saved successfully!\"}");
  } else {
    String resp = "{\"success\":false,\"error\":\"" + err + "\"}";
    _server.send(400, "application/json", resp);
  }
}

void WiFiManager::handleResetConfig() {
  _configModeStartTime = millis();
  String section = "all";
  if (_server.hasArg("section")) {
    section = _server.arg("section");
  }

  String err;
  bool ok = ConfigJsonHelper::resetSection(section, err);

  if (ok) {
    _server.send(200, "application/json", "{\"success\":true,\"message\":\"Reset to defaults successfully!\"}");
  } else {
    String resp = "{\"success\":false,\"error\":\"" + err + "\"}";
    _server.send(400, "application/json", resp);
  }
}

void WiFiManager::handleExitConfig() {
  _server.send(200, "application/json", "{\"success\":true,\"message\":\"Exiting configuration mode.\"}");
  // Allow HTTP response to flush before closing
  delay(100);
  exitConfigMode();
}

void WiFiManager::handleCaptivePortal() {
  _server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + String("/"), true);
  _server.send(302, "text/plain", "");
}
