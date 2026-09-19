#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "../config/BoardConfig.h"
#include "../config/LoRaConfig.h"
#include "../models/DisplayData.h"
#include "../time/TimeManager.h"

#include "../config/DevConfig.h"

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
//                 DISPLAY MODE ENUM
// =====================================================

enum DisplayMode {
  DISPLAY_MODE_NORMAL,      // Tank / Battery telemetry screens
  DISPLAY_MODE_SELECTION,   // Selection Menu (Tank Data, Config, Dev)
  DISPLAY_MODE_CONFIG,      // Config Mode screen
  DISPLAY_MODE_DEV          // Dev diagnostic screens (e.g. INA219)
};

enum SelectionOption {
  SELECT_OPT_TANK = 0,
  SELECT_OPT_CONFIG = 1,
  SELECT_OPT_DEV = 2
};

// =====================================================
//                 TEST / DEV SCREEN ENUM
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

  // Selection Screen & Navigation
  DisplayMode getDisplayMode() const { return _displayMode; }
  bool isSelectionScreenActive() const { return _displayMode == DISPLAY_MODE_SELECTION; }
  void openSelectionScreen();
  void closeSelectionScreen();
  void nextSelectionItem();
  void confirmSelection();
  void drawSelectionScreen();

  // Button Action Handlers
  void handleShortPress();
  void handleLongPress();

  // Dev / Diagnostic Mode management
  void setDevMode(bool active);
  bool isDevMode() const;
  void switchTestScreen();
  TestScreen getCurrentTestScreen() const;
  void updateTestScreen();
  void updateTestScreen(const BatteryLedManager& batteryLed);

  // Backward compatibility alias for test mode
  void setTestMode(bool active) { setDevMode(active); }
  bool isTestMode() const { return isDevMode(); }

  // UI Timeout & Power Saving
  bool isAwake() const;
  void wakeDisplay();
  void sleepDisplay();
  void resetTimeout();

  // Charging Animation & Transition
  bool isChargingAnimationActive() const;
  void startChargingAnimation();
  void checkChargingTransition(bool currentlyCharging);
  void drawChargingAnimation();

  void drawCurrentScreen();
  void drawNotConnectedScreen();
  void drawTankScreen(float tankValue);
  void drawBatteryScreen(float batteryValue);
  void drawWiFiNudgeScreen(const String& ssid, const String& ip, const char* statusMsg = "Open IP in browser");
  void drawWiFiConnectedScreen(const String& ssid, const String& ip);
  void showWiFiNudge(const String& ssid, const String& ip, const char* statusMsg = "Open IP in browser");
  void showWiFiConnected(const String& ssid, const String& ip);
  void drawConfigScreen(const String& ssid, const String& url, const String& ip, unsigned long remainingSec);

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
  DisplayMode _displayMode;
  DisplayMode _lastOpenedScreen;
  TestScreen _currentTestScreen;

  // Selection Screen state
  uint8_t _selectionIndex;
  uint8_t _selectionCount;
  SelectionOption _availableOptions[3];

  // Power save and UI timeout
  bool _displayAwake;
  unsigned long _lastUiActivityTime;
  unsigned long _uiTimeoutMs;

  // Charging transition & animation
  bool _chargingAnimationActive;
  unsigned long _chargingAnimationStart;
  Page _savedPageBeforeAnimation;
  TestScreen _savedTestScreenBeforeAnimation;
  DisplayMode _savedModeBeforeAnimation;
  bool _lastChargingState;
  bool _wasConfigModeActive;

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

