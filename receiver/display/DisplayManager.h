#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "../config/BoardConfig.h"
#include "../config/LoRaConfig.h"
#include "../config/DisplayConfig.h"
#include "../config/DisplayLayoutConfig.h"
#include "../models/DisplayData.h"
#include "../time/TimeManager.h"
#include "../config/DevConfig.h"

// Forward declaration
class BatteryLedManager;

// =====================================================
//                 BACKWARD COMPATIBILITY ENUMS
// =====================================================

enum Page {
  PAGE_TANK,
  PAGE_BATTERY
};

enum DisplayMode {
  DISPLAY_MODE_NORMAL,      // Tank / Battery telemetry screens
  DISPLAY_MODE_SELECTION,   // Selection Menu
  DISPLAY_MODE_CONFIG,      // Config Mode screen
  DISPLAY_MODE_DEV          // Dev diagnostic screens (e.g. INA219)
};

enum SelectionOption {
  SELECT_OPT_TANK = 0,
  SELECT_OPT_CONFIG = 1,
  SELECT_OPT_DEV = 2
};

enum TestScreen {
  TEST_SCREEN_INA219 = 0,
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
  void switchPage() { nextSectionPage(); }

  // Generic Section & Page Navigation
  SectionId getSection() const { return _currentSectionId; }
  void setSection(SectionId newSection, uint8_t pageIndex = 0);
  uint8_t getPageIndex() const { return _currentPageIndex; }
  void nextSectionPage();
  bool isPageAvailable(PageId pageId) const;

  const SectionDef* getSectionDef(SectionId id) const;
  const SectionDef* getCurrentSectionDef() const;
  PageId getCurrentPageId() const;

  // Selection Screen & Navigation
  DisplayMode getDisplayMode() const {
    if (_selectionScreenActive) return DISPLAY_MODE_SELECTION;
    if (_currentSectionId == SECTION_CONFIG) return DISPLAY_MODE_CONFIG;
    if (_currentSectionId == SECTION_DEV) return DISPLAY_MODE_DEV;
    return DISPLAY_MODE_NORMAL;
  }
  bool isSelectionScreenActive() const { return _selectionScreenActive; }
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
  bool isDevMode() const { return (_currentSectionId == SECTION_DEV && !_selectionScreenActive); }
  void switchTestScreen() { nextSectionPage(); }
  TestScreen getCurrentTestScreen() const { return (TestScreen)_currentPageIndex; }
  void updateTestScreen();
  void updateTestScreen(const BatteryLedManager& batteryLed);

  // Backward compatibility aliases
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

  // Generic and Concrete Drawing Methods
  void drawCurrentScreen();
  void drawSection(SectionId secId);
  void drawPage(PageId pageId);
  void updateSection(SectionId secId, unsigned long now);

  void drawNotConnectedScreen();
  void drawTankScreen(float tankValue);
  void drawBatteryScreen(float batteryValue);
  void drawWiFiNudgeScreen(const String& ssid, const String& ip, const char* statusMsg = "Open IP in browser");
  void drawWiFiConnectedScreen(const String& ssid, const String& ip);
  void showWiFiNudge(const String& ssid, const String& ip, const char* statusMsg = "Open IP in browser");
  void showWiFiConnected(const String& ssid, const String& ip);
  void drawConfigScreen(const String& ssid, const String& url, const String& ip, unsigned long remainingSec);
  void showPrompt(const char* line1, const char* line2 = nullptr);
  void drawPromptScreen();
  bool isPromptActive() const { return _promptActive; }

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

  // Generic Section & Page State
  SectionId _currentSectionId;
  SectionId _lastOpenedSection;
  uint8_t   _currentPageIndex;
  bool      _selectionScreenActive;

  // Selection Screen State
  uint8_t   _selectionIndex;
  uint8_t   _selectionCount;
  SectionId _availableSectionIds[DISPLAY_SECTION_COUNT];

  // Per-section update timers
  unsigned long _sectionLastUpdateMs[SECTION_COUNT];

  // Power save and UI timeout
  bool _displayAwake;
  unsigned long _lastUiActivityTime;
  unsigned long _uiTimeoutMs;

  // Charging transition & animation
  bool _chargingAnimationActive;
  unsigned long _chargingAnimationStart;
  SectionId _savedSectionBeforeAnimation;
  uint8_t   _savedPageIndexBeforeAnimation;
  bool      _savedSelectionActiveBeforeAnimation;
  bool _lastChargingState;
  bool _wasConfigModeActive;

  // Setup prompt popup notification
  bool          _promptActive;
  unsigned long _promptStartTime;
  char          _promptLine1[24];
  char          _promptLine2[24];

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
