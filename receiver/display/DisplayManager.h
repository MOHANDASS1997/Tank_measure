#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "../config/BoardConfig.h"
#include "../config/LoRaConfig.h"
#include "../models/DisplayData.h"
#include "../time/TimeManager.h"

// Forward declaration
class BatteryLedManager;

// =====================================================
//                    PAGE STATE
// =====================================================

enum Page {
  PAGE_TANK,
  PAGE_BATTERY
};

// =====================================================
//                 TEST SCREEN ENUM
// =====================================================
// To add a new test screen in the future:
// 1. Add an enum value before TEST_SCREEN_COUNT
// 2. Add a draw<Name>TestScreen() method
// 3. Add a case in updateTestScreen()
enum TestScreen {
  TEST_SCREEN_INA219 = 0,
  // Future test screens:
  // TEST_SCREEN_LORA,
  // TEST_SCREEN_WIFI,
  TEST_SCREEN_COUNT
};

// =====================================================
//                 DISPLAY MANAGER
// =====================================================

class DisplayManager {
public:
  DisplayManager();

  void begin();
  void update();
  void updateData(const DisplayData& data);
  void showNotConnected();
  void switchPage();

  // Test Mode management
  void setTestMode(bool active);
  bool isTestMode() const;
  void switchTestScreen();
  TestScreen getCurrentTestScreen() const;
  void updateTestScreen(const BatteryLedManager& batteryLed);

  void drawCurrentScreen();
  void drawNotConnectedScreen();
  void drawTankScreen(float tankValue);
  void drawBatteryScreen(float batteryValue);
  void drawWiFiNudgeScreen(const String& ssid, const String& ip, const char* statusMsg = "Open IP in browser");
  void drawWiFiConnectedScreen(const String& ssid, const String& ip);
  void showWiFiNudge(const String& ssid, const String& ip, const char* statusMsg = "Open IP in browser");
  void showWiFiConnected(const String& ssid, const String& ip);

  void drawIna219TestScreen(const BatteryLedManager& batteryLed);
  void drawIna219TestScreen(
    float busV,
    float shuntMv,
    float loadV,
    float currentMa,
    float powerMw,
    float batPct,
    const char* stateStr,
    bool isConnected,
    bool led1,
    bool led2,
    bool led3,
    bool led4,
    bool led5
  );

private:
  U8G2_SH1106_128X64_NONAME_F_HW_I2C _display;

  DisplayData _displayData;
  Page _currentPage;
  bool _testMode;
  TestScreen _currentTestScreen;

  int _screenW;
  int _screenH;
  int _minDim;

  float _displayedTank;
  float _displayedBattery;

  float _tankStart;
  float _tankTarget;
  float _batteryStart;
  float _batteryTarget;

  unsigned long _animationStart;
  static const unsigned long ANIMATION_DURATION = 500;
  bool _animationActive;

  void setSmallFont();
  void setLabelFont();
  void setLargeFont();

  void drawTextTop(int x, int top, const char* text);
  void drawLargePercentage(int value, int x, int y);
  void drawFooter();

  float smoothStep(float value);
  void startAnimation();
  void updateAnimation();

  float _waitAnimAngle;
  unsigned long _lastWaitAnimUpdate;
  static const unsigned long WAIT_ANIM_INTERVAL_MS = 250;

  unsigned long _lastFooterUpdate;

  void drawWaitingAnimation(int centerX, int centerY);
};

extern DisplayManager displayManager;
