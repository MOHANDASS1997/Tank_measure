#pragma once

// =====================================================
//                     WIFI CONFIG
// =====================================================

// Optional hardcoded Wi-Fi Station (STA) credentials.
// If left empty (""), the receiver will automatically use credentials
// saved in non-volatile flash (NVS) configured through the web portal.
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

// Wi-Fi Access Point (SoftAP) setup credentials
#define WIFI_AP_SSID "DASSHOME-Setup"
#define WIFI_AP_PASSWORD "" // Leave empty for open network

// Timeouts in milliseconds
#define WIFI_CONNECT_TIMEOUT_MS 8000
#define WIFI_CONNECTED_DISPLAY_MS 3000
