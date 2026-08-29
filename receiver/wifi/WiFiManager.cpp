#include "WiFiManager.h"

// Instantiate global WiFiManager
WiFiManager wifiManager;

WiFiManager::WiFiManager()
  : _connected(false),
    _isAP(false),
    _ip("0.0.0.0"),
    _ssid(""),
    _newSSID(""),
    _newPassword(""),
    _pendingConnect(false),
    _server(80) {
}

void WiFiManager::loadCredentials(String& ssid, String& password) {

  _preferences.begin("dasshome_wifi", true);
  ssid = _preferences.getString("ssid", "");
  password = _preferences.getString("pass", "");
  _preferences.end();

  // If not in NVS, fallback to WiFiConfig.h defaults if provided
  if (ssid.length() == 0 && strlen(WIFI_SSID) > 0) {
    ssid = String(WIFI_SSID);
    password = String(WIFI_PASSWORD);
  }
}

void WiFiManager::saveCredentials(const String& ssid, const String& password) {

  _preferences.begin("dasshome_wifi", false);
  _preferences.putString("ssid", ssid);
  _preferences.putString("pass", password);
  _preferences.end();

  Serial.println("Wi-Fi credentials saved to NVS storage.");
}

bool WiFiManager::tryConnectSTA(
  const char* ssid,
  const char* password,
  unsigned long timeoutMs
) {

  if (ssid == nullptr || strlen(ssid) == 0) {
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi SSID: ");
  Serial.print(ssid);

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startTime < timeoutMs) {
    delay(250);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    _connected = true;
    _isAP = false;
    _ip = WiFi.localIP().toString();
    _ssid = String(ssid);
    return true;
  }

  return false;
}

void WiFiManager::startAP() {

  WiFi.mode(WIFI_AP_STA);

  if (strlen(WIFI_AP_PASSWORD) > 0) {
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
  } else {
    WiFi.softAP(WIFI_AP_SSID);
  }

  _connected = false;
  _isAP = true;
  _ip = WiFi.softAPIP().toString();
  _ssid = String(WIFI_AP_SSID);
}

void WiFiManager::handleCaptivePortal() {

  _server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + String("/"), true);
  _server.send(302, "text/plain", "");
}

void WiFiManager::handleRoot() {

  int numNetworks = WiFi.scanNetworks();

  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1.0'>";
  html += "<title>DASS HOME Wi-Fi Setup</title>";
  html += "<style>";
  html += "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,Helvetica,Arial,sans-serif;";
  html += "background:#0f172a;color:#f8fafc;display:flex;justify-content:center;align-items:center;min-height:100vh;margin:0;padding:16px;box-sizing:border-box;}";
  html += ".card{background:#1e293b;border-radius:16px;box-shadow:0 10px 25px rgba(0,0,0,0.5);width:100%;max-width:380px;padding:28px;border:1px solid #334155;}";
  html += "h1{font-size:22px;margin:0 0 8px;color:#38bdf8;text-align:center;font-weight:700;}";
  html += "p{font-size:14px;color:#94a3b8;margin:0 0 24px;text-align:center;line-height:1.4;}";
  html += "label{display:block;font-size:13px;font-weight:600;margin-bottom:6px;color:#cbd5e1;}";
  html += "select,input{width:100%;padding:12px;margin-bottom:18px;background:#0f172a;border:1px solid #475569;border-radius:8px;color:#f8fafc;font-size:15px;box-sizing:border-box;outline:none;}";
  html += "select:focus,input:focus{border-color:#38bdf8;}";
  html += "button{width:100%;padding:13px;background:#0284c7;color:#fff;border:none;border-radius:8px;font-size:16px;font-weight:600;cursor:pointer;transition:background 0.2s;}";
  html += "button:hover{background:#0369a1;}";
  html += ".badge{font-size:11px;background:#334155;color:#94a3b8;padding:4px 8px;border-radius:12px;float:right;}";
  html += "</style>";
  html += "<script>";
  html += "function selectSSID(val){if(val){document.getElementById('customSSID').value=val;}}";
  html += "</script></head><body>";
  html += "<div class='card'>";
  html += "<h1>DASS HOME Wi-Fi</h1>";
  html += "<p>Configure Wi-Fi connection for your receiver.</p>";
  html += "<form method='POST' action='/save'>";

  if (numNetworks > 0) {
    html += "<label>Detected Networks</label>";
    html += "<select onchange='selectSSID(this.value)'>";
    html += "<option value=''>-- Select your Wi-Fi --</option>";
    for (int i = 0; i < numNetworks; i++) {
      html += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)</option>";
    }
    html += "</select>";
  }

  html += "<label>Network Name (SSID)</label>";
  html += "<input type='text' id='customSSID' name='ssid' placeholder='Enter Wi-Fi SSID' required>";
  html += "<label>Wi-Fi Password</label>";
  html += "<input type='password' name='password' placeholder='Enter Password'>";
  html += "<button type='submit'>Save & Connect</button>";
  html += "</form></div></body></html>";

  _server.send(200, "text/html", html);
}

void WiFiManager::handleSave() {

  if (_server.hasArg("ssid")) {
    _newSSID = _server.arg("ssid");
    _newPassword = _server.arg("password");
    _pendingConnect = true;

    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width,initial-scale=1.0'>";
    html += "<title>Connecting...</title>";
    html += "<style>body{font-family:sans-serif;background:#0f172a;color:#f8fafc;display:flex;justify-content:center;align-items:center;min-height:100vh;margin:0;text-align:center;}";
    html += ".card{background:#1e293b;padding:32px;border-radius:16px;max-width:340px;border:1px solid #334155;}";
    html += "h1{color:#38bdf8;font-size:20px;}p{color:#94a3b8;line-height:1.5;}</style></head>";
    html += "<body><div class='card'>";
    html += "<h1>Connecting to Wi-Fi</h1>";
    html += "<p>Attempting connection to <b>" + _newSSID + "</b>.<br><br>Please check the OLED display on your DASS HOME receiver.</p>";
    html += "</div></body></html>";

    _server.send(200, "text/html", html);
  } else {
    _server.send(400, "text/plain", "Missing SSID parameter");
  }
}

void WiFiManager::setupWebServer() {

  _server.on("/", HTTP_GET, [this]() {
    handleRoot();
  });

  _server.on("/save", HTTP_POST, [this]() {
    handleSave();
  });

  // Captive portal redirect routes
  _server.on("/generate_204", [this]() { handleCaptivePortal(); });
  _server.on("/hotspot-detect.html", [this]() { handleCaptivePortal(); });
  _server.on("/canonical.html", [this]() { handleCaptivePortal(); });
  _server.on("/connecttest.txt", [this]() { handleCaptivePortal(); });
  _server.on("/ncsi.txt", [this]() { handleCaptivePortal(); });

  _server.onNotFound([this]() {
    handleCaptivePortal();
  });

  _server.begin();
  Serial.println("Wi-Fi Setup Web Server started on port 80.");
}

void WiFiManager::begin() {

  Serial.println();
  Serial.println("================================");
  Serial.println("Wi-Fi Status & Connection Check");
  Serial.println("================================");

  String storedSSID = "";
  String storedPassword = "";
  loadCredentials(storedSSID, storedPassword);

  bool connected = false;

  if (storedSSID.length() > 0) {
    Serial.print("Attempting connection to saved network: ");
    Serial.println(storedSSID);
    connected = tryConnectSTA(storedSSID.c_str(), storedPassword.c_str(), WIFI_CONNECT_TIMEOUT_MS);
  }

  if (connected) {
    Serial.println("Wi-Fi connected successfully!");
    Serial.print("SSID: ");
    Serial.println(_ssid);
    Serial.print("IP Address: ");
    Serial.println(_ip);
  } else {
    Serial.println("Wi-Fi not connected. Starting Access Point for setup.");
    startAP();
    Serial.print("AP SSID: ");
    Serial.println(_ssid);
    Serial.print("AP IP Address: ");
    Serial.println(_ip);
  }

  Serial.println("================================");
}

void WiFiManager::runStartupFlow(
  DisplayManager& display,
  ButtonManager& button
) {

  if (_connected) {
    // Wi-Fi already connected: directly proceed to subsequent screens
    return;
  }

  // If not connected, start WebServer and DNS Server for setup
  _dnsServer.start(53, "*", WiFi.softAPIP());
  setupWebServer();

  // Show nudge screen prompting user to connect to WiFi and showing IP
  display.showWiFiNudge(_ssid, _ip, "Open IP in browser");

  Serial.println("Wi-Fi setup screen active. Waiting for setup to complete...");

  // BLOCKING LOOP: Do not dismiss the screen until Wi-Fi setup is complete
  while (!_connected) {

    _dnsServer.processNextRequest();
    _server.handleClient();

    if (_pendingConnect) {
      display.showWiFiNudge(_ssid, _ip, "Connecting to WiFi...");
      Serial.print("Connecting to new Wi-Fi: ");
      Serial.println(_newSSID);

      bool ok = tryConnectSTA(_newSSID.c_str(), _newPassword.c_str(), WIFI_CONNECT_TIMEOUT_MS);

      if (ok) {
        Serial.println("Wi-Fi connected successfully!");
        Serial.print("Assigned IP: ");
        Serial.println(_ip);

        saveCredentials(_newSSID, _newPassword);

        // Stop setup services
        _server.stop();
        _dnsServer.stop();
        WiFi.softAPdisconnect(true);

        break; // Setup complete! Proceed directly to subsequent screens
      } else {
        Serial.println("Connection failed. Returning to AP setup mode.");
        startAP();
        _dnsServer.start(53, "*", WiFi.softAPIP());
        _server.begin();
        display.showWiFiNudge(_ssid, _ip, "Connect failed, retry");
        _pendingConnect = false;
      }
    }

    delay(10);
  }

  Serial.println("Wi-Fi setup completed. Proceeding to LoRa setup.");
}

bool WiFiManager::isConnected() const {
  return _connected;
}

bool WiFiManager::isAPActive() const {
  return _isAP;
}

String WiFiManager::getIP() const {
  return _ip;
}

String WiFiManager::getSSID() const {
  return _ssid;
}
