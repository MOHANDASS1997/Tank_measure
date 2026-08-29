#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "../config/BoardConfig.h"
#include "../config/LoRaConfig.h"
#include "../models/DisplayData.h"

// =====================================================
//                    PAGE STATE
// =====================================================

enum Page {
  PAGE_TANK,
  PAGE_BATTERY
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

  void drawCurrentScreen();
  void drawNotConnectedScreen();
  void drawTankScreen(float tankValue);
  void drawBatteryScreen(float batteryValue);

private:
  U8G2_SH1106_128X64_NONAME_F_HW_I2C _display;

  DisplayData _displayData;
  Page _currentPage;

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
  void drawPageIndicator();

  float smoothStep(float value);
  void startAnimation();
  void updateAnimation();
};

extern DisplayManager displayManager;
