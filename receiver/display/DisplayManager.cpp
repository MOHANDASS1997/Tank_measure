#include "DisplayManager.h"
#include "../battery/BatteryLedManager.h"
#include "../wifi/WiFiManager.h"
#include "../config/SystemConfig.h"

// Instantiate global DisplayManager
DisplayManager displayManager;

DisplayManager::DisplayManager()
  : _display(U8G2_R0, U8X8_PIN_NONE),
    _currentPage(PAGE_TANK),
    _displayMode(DISPLAY_MODE_NORMAL),
    _lastOpenedScreen(DISPLAY_MODE_NORMAL),
    _currentTestScreen(TEST_SCREEN_INA219),
    _selectionIndex(0),
    _selectionCount(2),
    _displayAwake(true),
    _lastUiActivityTime(0),
    _uiTimeoutMs(UI_TIMEOUT_MS),
    _chargingAnimationActive(false),
    _chargingAnimationStart(0),
    _savedPageBeforeAnimation(PAGE_TANK),
    _savedTestScreenBeforeAnimation(TEST_SCREEN_INA219),
    _savedModeBeforeAnimation(DISPLAY_MODE_NORMAL),
    _lastChargingState(false),
    _wasConfigModeActive(false),
    _screenW(128),
    _screenH(64),
    _minDim(64),
    _displayedTank(0.0),
    _displayedBattery(0.0),
    _tankStart(0.0),
    _tankTarget(0.0),
    _batteryStart(0.0),
    _batteryTarget(0.0),
    _animationStart(0),
    _animationActive(false),
    _waitAnimAngle(0.0f),
    _lastWaitAnimUpdate(0),
    _lastFooterUpdate(0) {

  _displayData.valid = false;
  _displayData.hasBattery = false;
  _displayData.timestamp = 0;
  _displayData.transmitterAddress = 0;
  _displayData.tankId = "";
  _displayData.distanceCm = 0.0;
  _displayData.tankPercent = 0.0;
  _displayData.currentLitres = 0.0;
  _displayData.capacityLitres = 0.0;
  _displayData.batteryPercent = 0.0;
  _displayData.batteryVoltage = 0.0;
  _displayData.charging = false;
  _displayData.sequence = 0;
  _displayData.lastReceived = 0;
  _displayData.rssi = 0;
  _displayData.snr = 0;
}

// =====================================================
//                 INITIALIZATION
// =====================================================

void DisplayManager::begin() {

  Wire.begin(
    OLED_SDA,
    OLED_SCL
  );

  Wire.setClock(
    400000
  );

  _display.begin();

  _screenW =
    _display.getDisplayWidth();

  _screenH =
    _display.getDisplayHeight();

  _minDim =
    min(
      _screenW,
      _screenH
    );

  Serial.print(
    "OLED: "
  );

  Serial.print(
    _screenW
  );

  Serial.print(
    " x "
  );

  Serial.println(
    _screenH
  );

  _lastUiActivityTime = millis();
  _lastChargingState = batteryLedManager.isCharging();
}

// =====================================================
//                     FONT HELPERS
// =====================================================

void DisplayManager::setSmallFont() {

  _display.setFont(
    u8g2_font_5x8_tr
  );
}

void DisplayManager::setLabelFont() {

  _display.setFont(
    u8g2_font_6x10_tr
  );
}

void DisplayManager::setLargeFont() {

  _display.setFont(
    u8g2_font_logisoso24_tn
  );
}

// =====================================================
// Text helper
// =====================================================

void DisplayManager::drawTextTop(
  int x,
  int top,
  const char* text
) {

  int baseline =
    top +
    _display.getAscent();

  _display.drawStr(
    x,
    baseline,
    text
  );
}

// =====================================================
//                 LARGE PERCENTAGE
// =====================================================

void DisplayManager::drawLargePercentage(
  int value,
  int x,
  int y
) {

  setLargeFont();

  String number =
    String(
      constrain(
        value,
        0,
        100
      )
    );

  int numberAscent =
    _display.getAscent();

  int numberDescent =
    _display.getDescent();

  int numberHeight =
    numberAscent -
    numberDescent;

  _display.drawStr(
    x,
    y + numberAscent,
    number.c_str()
  );

  int numberWidth =
    _display.getStrWidth(
      number.c_str()
    );

  // Numeric font doesn't contain %
  _display.setFont(
    u8g2_font_10x20_tr
  );

  int percentAscent =
    _display.getAscent();

  int percentDescent =
    _display.getDescent();

  int percentHeight =
    percentAscent -
    percentDescent;

  int percentX =
    x +
    numberWidth +
    max(
      2,
      (int)round(
        _minDim * 0.02
      )
    );

  int percentTop =
    y +
    max(
      0,
      (
        numberHeight -
        percentHeight
      ) / 2
    );

  _display.drawStr(
    percentX,
    percentTop +
      percentAscent,
    "%"
  );
}

// =====================================================
//                     ANIMATION
// =====================================================

float DisplayManager::smoothStep(
  float value
) {

  value =
    constrain(
      value,
      0.0,
      1.0
    );

  return
    value *
    value *
    (
      3.0 -
      2.0 *
      value
    );
}

void DisplayManager::startAnimation() {

  _tankStart =
    _displayedTank;

  _tankTarget =
    _displayData.tankPercent;

  _batteryStart =
    _displayedBattery;

  _batteryTarget =
    _displayData.batteryPercent;

  _animationStart =
    millis();

  _animationActive =
    true;
}

void DisplayManager::updateAnimation() {

  if (
    !_animationActive
  ) {

    return;
  }

  float progress =
    (float)(
      millis() -
      _animationStart
    ) /
    ANIMATION_DURATION;

  if (
    progress >= 1.0
  ) {

    progress =
      1.0;

    _animationActive =
      false;
  }

  progress =
    smoothStep(
      progress
    );

  _displayedTank =
    _tankStart +
    (
      _tankTarget -
      _tankStart
    ) *
    progress;

  _displayedBattery =
    _batteryStart +
    (
      _batteryTarget -
      _batteryStart
    ) *
    progress;
}

// =====================================================
//                       FOOTER
// =====================================================

void DisplayManager::drawFooter() {

  // ---------------------------------------------------
  // Left: Elongated Curved Rectangle Page Indicator
  // (Only if battery data exists)
  // ---------------------------------------------------
  if (_displayData.hasBattery) {
    int pillY = _screenH - 5;
    int pillH = 4;
    int pillR = 1;

    if (_currentPage == PAGE_TANK) {
      // Tank active (elongated pill on left, small pill on right)
      _display.drawRBox(4, pillY, 10, pillH, pillR);
      _display.drawRBox(17, pillY, 4, pillH, pillR);
    } else {
      // Battery active (small pill on left, elongated pill on right)
      _display.drawRBox(4, pillY, 4, pillH, pillR);
      _display.drawRBox(11, pillY, 10, pillH, pillR);
    }
  }

  // ---------------------------------------------------
  // Right: Elapsed Time (<duration> ago)
  // Completely bottom-aligned along the display edge
  // ---------------------------------------------------
  String elapsed = timeManager.formatElapsed(_displayData.timestamp, _displayData.lastReceived);

  setSmallFont();
  int textWidth = _display.getStrWidth(elapsed.c_str());
  int textX = _screenW - textWidth;

  // Draw elapsed time text
  _display.drawStr(textX, _screenH - 1, elapsed.c_str());
}

// =====================================================
//             HOURGLASS WAITING ANIMATION
// =====================================================

void DisplayManager::drawWaitingAnimation(
  int centerX,
  int centerY
) {

  // Ambient time sparkles in background
  _display.drawPixel(centerX - 13, centerY - 14);
  _display.drawPixel(centerX + 14, centerY - 12);
  _display.drawPixel(centerX - 12, centerY + 14);
  _display.drawPixel(centerX + 13, centerY + 13);

  if (_waitAnimAngle < 270.0f) {
    // -------------------------------------------------
    // Phase 1: Sand trickling down (0° - 270°)
    // -------------------------------------------------
    float progress = _waitAnimAngle / 270.0f; // 0.0 to 1.0

    // Top & Bottom horizontal plates
    _display.drawHLine(centerX - 8, centerY - 13, 17);
    _display.drawHLine(centerX - 8, centerY + 13, 17);

    // Glass contours
    _display.drawLine(centerX - 7, centerY - 12, centerX - 2, centerY - 1);
    _display.drawLine(centerX + 7, centerY - 12, centerX + 2, centerY - 1);
    _display.drawLine(centerX - 2, centerY + 1, centerX - 7, centerY + 12);
    _display.drawLine(centerX + 2, centerY + 1, centerX + 7, centerY + 12);
    _display.drawLine(centerX - 2, centerY - 1, centerX - 2, centerY + 1);
    _display.drawLine(centerX + 2, centerY - 1, centerX + 2, centerY + 1);

    // Upper chamber sand (draining)
    int topLines = (int)round((1.0f - progress) * 5.0f);
    for (int i = 0; i < topLines; i++) {
      int y = centerY - 3 - i * 2;
      int halfW = map(i, 0, 5, 2, 6);
      _display.drawHLine(centerX - halfW, y, 2 * halfW + 1);
    }

    // Flowing sand stream in the center neck
    _display.drawVLine(centerX, centerY - 1, 6);
    int grain = ((int)(_waitAnimAngle * 1.5f)) % 6;
    _display.drawPixel(centerX, centerY + grain);

    // Lower chamber sand (accumulating)
    int botLines = (int)round(progress * 5.0f);
    for (int i = 0; i < botLines; i++) {
      int y = centerY + 11 - i * 2;
      int halfW = map(i, 0, 5, 6, 2);
      _display.drawHLine(centerX - halfW, y, 2 * halfW + 1);
    }

  } else {
    // -------------------------------------------------
    // Phase 2: Smooth 180° Hourglass Flip (270° - 360°)
    // -------------------------------------------------
    float flipProgress = (_waitAnimAngle - 270.0f) / 90.0f; // 0.0 to 1.0
    float phi = flipProgress * PI;                          // 0 to PI

    float cosA = cos(phi);
    float sinA = sin(phi);

    // 8 key vertices defining the hourglass shape
    const int numV = 8;
    const int vx[numV] = { -8,  8, -8,  8, -2,  2, -2,  2 };
    const int vy[numV] = {-13,-13, 13, 13, -1, -1,  1,  1 };

    int rx[numV];
    int ry[numV];

    for (int i = 0; i < numV; i++) {
      rx[i] = centerX + (int)round(vx[i] * cosA - vy[i] * sinA);
      ry[i] = centerY + (int)round(vx[i] * sinA + vy[i] * cosA);
    }

    // Top and bottom plates
    _display.drawLine(rx[0], ry[0], rx[1], ry[1]);
    _display.drawLine(rx[2], ry[2], rx[3], ry[3]);

    // Upper sides
    _display.drawLine(rx[0], ry[0], rx[4], ry[4]);
    _display.drawLine(rx[1], ry[1], rx[5], ry[5]);

    // Neck
    _display.drawLine(rx[4], ry[4], rx[6], ry[6]);
    _display.drawLine(rx[5], ry[5], rx[7], ry[7]);

    // Lower sides
    _display.drawLine(rx[6], ry[6], rx[2], ry[2]);
    _display.drawLine(rx[7], ry[7], rx[3], ry[3]);

    // Tumbling sand particle core during flip
    _display.drawDisc(centerX, centerY, 2);
  }
}

// =====================================================
//                 NOT CONNECTED SCREEN
// =====================================================

void DisplayManager::drawNotConnectedScreen() {

  if (!_displayAwake || _chargingAnimationActive) {
    return;
  }

  _display.clearBuffer();

  // ---------------------------------------------------
  // Virtual 60:40 Compartment Split (Borderless)
  // Left: ~60% (0..76 px), Right: ~40% (77..127 px)
  // ---------------------------------------------------

  // Left Compartment: Header & Waiting Text
  setLabelFont();
  const char* title = "DASS HOME";
  drawTextTop(6, 10, title);

  setSmallFont();
  const char* line1 = "Waiting for";
  drawTextTop(6, 28, line1);

  const char* line2 = "tank data...";
  drawTextTop(6, 40, line2);

  // Right Compartment: Animated Hourglass Waiting Indicator
  int rightCenterX = 77 + (128 - 77) / 2; // = 102
  int rightCenterY = _screenH / 2;         // = 32

  drawWaitingAnimation(rightCenterX, rightCenterY);

  _display.sendBuffer();
}

// =====================================================
//                 WIFI NUDGE SCREEN
// =====================================================

void DisplayManager::drawWiFiNudgeScreen(
  const String& ssid,
  const String& ip,
  const char* statusMsg
) {

  _display.clearBuffer();

  // Header
  setLabelFont();
  const char* title = "WIFI SETUP";
  int titleWidth = _display.getStrWidth(title);
  drawTextTop(
    (_screenW - titleWidth) / 2,
    2,
    title
  );

  _display.drawHLine(6, 14, _screenW - 12);

  // Body
  setSmallFont();
  drawTextTop(8, 17, "Connect to Hotspot:");

  String ssidLine = "SSID: " + ssid;
  drawTextTop(8, 27, ssidLine.c_str());

  setLabelFont();
  String ipLine = "IP: " + ip;
  drawTextTop(8, 38, ipLine.c_str());

  // Footer
  _display.drawHLine(6, 50, _screenW - 12);

  setSmallFont();
  const char* hint = (statusMsg != nullptr) ? statusMsg : "Open IP in browser";
  int hintWidth = _display.getStrWidth(hint);
  drawTextTop(
    (_screenW - hintWidth) / 2,
    53,
    hint
  );

  _display.sendBuffer();
}

// =====================================================
//               WIFI CONNECTED SCREEN
// =====================================================

void DisplayManager::drawWiFiConnectedScreen(
  const String& ssid,
  const String& ip
) {

  _display.clearBuffer();

  // Header
  setLabelFont();
  const char* title = "WIFI CONNECTED";
  int titleWidth = _display.getStrWidth(title);
  drawTextTop(
    (_screenW - titleWidth) / 2,
    2,
    title
  );

  _display.drawHLine(6, 14, _screenW - 12);

  // Body
  setSmallFont();
  String ssidLine = "SSID: " + ssid;
  drawTextTop(8, 18, ssidLine.c_str());

  drawTextTop(8, 29, "Assigned IP:");

  setLabelFont();
  drawTextTop(8, 39, ip.c_str());

  // Footer
  _display.drawHLine(6, 51, _screenW - 12);

  setSmallFont();
  const char* status = "Starting LoRa...";
  int statusWidth = _display.getStrWidth(status);
  drawTextTop(
    (_screenW - statusWidth) / 2,
    54,
    status
  );

  _display.sendBuffer();
}

// =====================================================
//                     TANK SCREEN
// =====================================================

void DisplayManager::drawTankScreen(
  float tankValue
) {

  _display.clearBuffer();

  int margin =
    max(
      2,
      (int)round(
        _minDim * 0.025
      )
    );

  int footerHeight =
    max(
      10,
      (int)round(
        _screenH * 0.16
      )
    );

  int contentTop =
    margin;

  int contentBottom =
    _screenH -
    footerHeight;

  int contentHeight =
    contentBottom -
    contentTop;

  // ---------------------------------------------------
  // Bar dimensions
  // ---------------------------------------------------

  int scaleWidth =
    (int)round(
      _screenW * 0.14
    );

  int barWidth =
    max(
      10,
      (int)round(
        _screenW * 0.16
      )
    );

  int scaleBarGap =
    max(
      2,
      (int)round(
        _screenW * 0.01
      )
    );

  int barInfoGap =
    max(
      3,
      (int)round(
        _screenW * 0.035
      )
    );

  int barX =
    margin +
    scaleWidth +
    scaleBarGap;

  int barY =
    contentTop;

  int barHeight =
    contentHeight;

  // ---------------------------------------------------
  // Scale
  // ---------------------------------------------------

  setSmallFont();

  int tickRight =
    barX -
    scaleBarGap;

  int tickLength =
    max(
      4,
      (int)round(
        _screenW * 0.045
      )
    );

  for (
    int level = 25;
    level <= 75;
    level += 25
  ) {

    int y =
      barY +
      barHeight -
      (int)round(
        barHeight *
        level /
        100.0
      );

    _display.drawHLine(
      tickRight -
        tickLength,
      y,
      tickLength
    );
  }

  // ---------------------------------------------------
  // 100
  // ---------------------------------------------------

  String topLabel =
    "100";

  int topLabelWidth =
    _display.getStrWidth(
      topLabel.c_str()
    );

  drawTextTop(
    tickRight -
      topLabelWidth,
    barY +
      max(
        1,
        (int)round(
          _screenH * 0.015
        )
      ),
    topLabel.c_str()
  );

  // ---------------------------------------------------
  // 0
  // ---------------------------------------------------

  String bottomLabel =
    "0";

  int bottomLabelWidth =
    _display.getStrWidth(
      bottomLabel.c_str()
    );

  int bottomTextHeight =
    _display.getAscent() -
    _display.getDescent();

  drawTextTop(
    tickRight -
      bottomLabelWidth,
    barY +
      barHeight -
      bottomTextHeight,
    bottomLabel.c_str()
  );

  // ---------------------------------------------------
  // Tank bar
  // ---------------------------------------------------

  _display.drawFrame(
    barX,
    barY,
    barWidth,
    barHeight
  );

  int border =
    max(
      1,
      (int)round(
        _minDim * 0.012
      )
    );

  int innerX =
    barX +
    border;

  int innerY =
    barY +
    border;

  int innerWidth =
    max(
      1,
      barWidth -
      2 * border
    );

  int innerHeight =
    max(
      1,
      barHeight -
      2 * border
    );

  float safeTank =
    constrain(
      tankValue,
      0.0,
      100.0
    );

  int fillHeight =
    (int)round(
      innerHeight *
      safeTank /
      100.0
    );

  if (
    fillHeight > 0
  ) {

    _display.drawBox(
      innerX,
      innerY +
        innerHeight -
        fillHeight,
      innerWidth,
      fillHeight
    );
  }

  // ===================================================
  // Information area
  // ===================================================

  int infoX =
    barX +
    barWidth +
    barInfoGap;

  int infoRight =
    _screenW -
    margin;

  int infoWidth =
    max(
      1,
      infoRight -
      infoX
    );

  // ---------------------------------------------------
  // TANK
  // ---------------------------------------------------

  setLabelFont();

  const char* title =
    "TANK";

  int titleWidth =
    _display.getStrWidth(
      title
    );

  drawTextTop(
    infoX +
      max(
        0,
        (
          infoWidth -
          titleWidth
        ) / 2
      ),
    contentTop,
    title
  );

  int titleHeight =
    _display.getAscent() -
    _display.getDescent();

  int separatorY =
    contentTop +
    titleHeight +
    2;

  _display.drawHLine(
    infoX,
    separatorY,
    infoWidth
  );

  // ---------------------------------------------------
  // Percentage
  // ---------------------------------------------------

  int percentageValue =
    (int)round(
      constrain(
        tankValue,
        0.0,
        100.0
      )
    );

  setLargeFont();

  String percentageString =
    String(
      percentageValue
    );

  int percentageWidth =
    _display.getStrWidth(
      percentageString.c_str()
    );

  int percentageHeight =
    _display.getAscent() -
    _display.getDescent();

  int percentageTop =
    separatorY +
    4;

  int percentageX =
    infoX +
    max(
      0,
      (
        infoWidth -
        percentageWidth
      ) / 2
    );

  drawLargePercentage(
    percentageValue,
    percentageX,
    percentageTop
  );

  // ---------------------------------------------------
  // Litres
  // ---------------------------------------------------

  int litres =
    (int)round(
      _displayData.currentLitres
    );

  String litresText =
    String(
      litres
    ) +
    "L / " +
    String(
      (int)round(
        _displayData.capacityLitres
      )
    ) +
    "L";

  setLabelFont();

  int litresWidth =
    _display.getStrWidth(
      litresText.c_str()
    );

  int litresTop =
    percentageTop +
    percentageHeight;

  drawTextTop(
    infoX +
      max(
        0,
        (
          infoWidth -
          litresWidth
        ) / 2
      ),
    litresTop,
    litresText.c_str()
  );

  drawFooter();

  _display.sendBuffer();
}

// =====================================================
//                  BATTERY SCREEN
// =====================================================

void DisplayManager::drawBatteryScreen(
  float batteryValue
) {

  _display.clearBuffer();

  int margin =
    max(
      3,
      (int)round(
        _minDim * 0.05
      )
    );

  int footerHeight =
    max(
      10,
      (int)round(
        _screenH * 0.16
      )
    );

  int contentTop =
    margin;

  int contentBottom =
    _screenH -
    footerHeight;

  // ---------------------------------------------------
  // Battery bar
  // ---------------------------------------------------

  int barHeight =
    max(
      8,
      (int)round(
        _screenH * 0.14
      )
    );

  int barBottomGap =
    max(
      2,
      (int)round(
        _screenH * 0.025
      )
    );

  int barY =
    contentBottom -
    barHeight -
    barBottomGap;

  // ---------------------------------------------------
  // Upper area
  // ---------------------------------------------------

  int upperTop =
    contentTop;

  int upperBottom =
    barY;

  int upperHeight =
    upperBottom -
    upperTop;

  int upperMiddle =
    upperTop +
    upperHeight / 2;

  // ===================================================
  // LEFT COMPARTMENT
  // ===================================================

  int leftX =
    margin;

  int leftWidth =
    _screenW / 2 -
    margin;

  // ===================================================
  // LEFT TOP: VOLTAGE
  // ===================================================

  int voltageTop =
    upperTop;

  int voltageAreaHeight =
    upperMiddle -
    upperTop;

  setSmallFont();

  const char* voltageLabel =
    "VOLTAGE";

  int voltageLabelWidth =
    _display.getStrWidth(
      voltageLabel
    );

  int voltageLabelHeight =
    _display.getAscent() -
    _display.getDescent();

  int voltageLabelX =
    leftX +
    max(
      0,
      (
        leftWidth -
        voltageLabelWidth
      ) / 2
    );

  int voltageLabelTop =
    voltageTop +
    max(
      0,
      (
        voltageAreaHeight -
        voltageLabelHeight -
        max(
          2,
          (int)round(
            _screenH * 0.035
          )
        ) -
        10
      ) / 2
    );

  drawTextTop(
    voltageLabelX,
    voltageLabelTop,
    voltageLabel
  );

  // ---------------------------------------------------
  // Voltage value
  // ---------------------------------------------------

  setLabelFont();

  String voltage =
    String(
      _displayData.batteryVoltage,
      2
    ) +
    " V";

  int voltageWidth =
    _display.getStrWidth(
      voltage.c_str()
    );

  int voltageLabelActualHeight =
    _display.getAscent() -
    _display.getDescent();

  int voltageTopValue =
    voltageLabelTop +
    voltageLabelActualHeight +
    max(
      2,
      (int)round(
        _screenH * 0.035
      )
    );

  int voltageX =
    leftX +
    max(
      0,
      (
        leftWidth -
        voltageWidth
      ) / 2
    );

  drawTextTop(
    voltageX,
    voltageTopValue,
    voltage.c_str()
  );

  // ===================================================
  // LEFT BOTTOM: CHARGING
  // ===================================================

  if (
    _displayData.charging
  ) {

    setSmallFont();

    const char* chargingText =
      "CHARGING...";

    int chargingWidth =
      _display.getStrWidth(
        chargingText
      );

    int chargingHeight =
      _display.getAscent() -
      _display.getDescent();

    int chargingX =
      leftX +
      max(
        0,
        (
          leftWidth -
          chargingWidth
        ) / 2
      );

    int chargingTop =
      upperMiddle +
      max(
        0,
        (
          (
            upperBottom -
            upperMiddle
          ) -
          chargingHeight
        ) / 2
      );

    drawTextTop(
      chargingX,
      chargingTop,
      chargingText
    );
  }

  // ===================================================
  // RIGHT COMPARTMENT
  // ===================================================

  int rightX =
    _screenW / 2;

  int rightWidth =
    _screenW -
    rightX -
    margin;

  // ---------------------------------------------------
  // BATTERY TITLE
  // ---------------------------------------------------

  setSmallFont();

  const char* batteryTitle =
    "BATTERY";

  int batteryTitleWidth =
    _display.getStrWidth(
      batteryTitle
    );

  int batteryTitleHeight =
    _display.getAscent() -
    _display.getDescent();

  int batteryTitleX =
    rightX +
    max(
      0,
      (
        rightWidth -
        batteryTitleWidth
      ) / 2
    );

  drawTextTop(
    batteryTitleX,
    contentTop,
    batteryTitle
  );

  // ---------------------------------------------------
  // Battery percentage
  // ---------------------------------------------------

  int percentageValue =
    (int)round(
      constrain(
        batteryValue,
        0.0,
        100.0
      )
    );

  setLargeFont();

  String percentageString =
    String(
      percentageValue
    );

  int percentageWidth =
    _display.getStrWidth(
      percentageString.c_str()
    );

  int percentageHeight =
    _display.getAscent() -
    _display.getDescent();

  int percentageTop =
    contentTop +
    batteryTitleHeight +
    5;

  int percentageX =
    rightX +
    max(
      0,
      (
        rightWidth -
        percentageWidth
      ) / 2
    );

  drawLargePercentage(
    percentageValue,
    percentageX,
    percentageTop
  );

  // ===================================================
  // BATTERY BAR
  // ===================================================

  int barX =
    margin;

  int barWidth =
    _screenW -
    2 * margin;

  _display.drawFrame(
    barX,
    barY,
    barWidth,
    barHeight
  );

  int border =
    max(
      1,
      (int)round(
        _minDim * 0.012
      )
    );

  int innerWidth =
    max(
      1,
      barWidth -
      2 * border
    );

  int innerHeight =
    max(
      1,
      barHeight -
      2 * border
    );

  int fillWidth =
    (int)round(
      innerWidth *
      constrain(
        batteryValue,
        0.0,
        100.0
      ) /
      100.0
    );

  if (
    fillWidth > 0
  ) {

    _display.drawBox(
      barX +
        border,
      barY +
        border,
      fillWidth,
      innerHeight
    );
  }

  drawFooter();

  _display.sendBuffer();
}

// =====================================================
//                  UI CONTROLLER
// =====================================================

void DisplayManager::drawCurrentScreen() {

  if (!_displayAwake || _chargingAnimationActive) {
    return;
  }

  // Priority 1: Selection Screen (popup menu on long press)
  if (_displayMode == DISPLAY_MODE_SELECTION) {
    drawSelectionScreen();
    return;
  }

  // Priority 2: Configuration Mode Screen
  if (_displayMode == DISPLAY_MODE_CONFIG || wifiManager.isConfigModeActive()) {
    drawConfigScreen(wifiManager.getSSID(), wifiManager.getHostname(), wifiManager.getIP(), wifiManager.getConfigModeRemainingSeconds());
    return;
  }

  // Priority 3: Dev Mode Diagnostic Screen
  if (_displayMode == DISPLAY_MODE_DEV) {
    updateTestScreen();
    return;
  }

  if (
    !_displayData.valid
  ) {

    drawNotConnectedScreen();

    return;
  }

  // If battery data is not present, always stay on tank page
  if (
    !_displayData.hasBattery &&
    _currentPage == PAGE_BATTERY
  ) {
    _currentPage = PAGE_TANK;
  }

  if (
    _currentPage ==
    PAGE_TANK
  ) {

    drawTankScreen(
      _displayedTank
    );

  } else {

    drawBatteryScreen(
      _displayedBattery
    );
  }
}

// =====================================================
//                  PUBLIC METHODS
// =====================================================

void DisplayManager::updateData(const DisplayData& data) {

  _displayData = data;
  _lastFooterUpdate = millis();
  startAnimation();
  if (_displayAwake && !_chargingAnimationActive) {
    drawCurrentScreen();
  }
}

void DisplayManager::showNotConnected() {

  _displayData.valid = false;
  _animationActive = false;
  drawNotConnectedScreen();
}

void DisplayManager::showWiFiNudge(
  const String& ssid,
  const String& ip,
  const char* statusMsg
) {

  _displayData.valid = false;
  _animationActive = false;
  drawWiFiNudgeScreen(
    ssid,
    ip,
    statusMsg
  );
}

void DisplayManager::showWiFiConnected(
  const String& ssid,
  const String& ip
) {

  _displayData.valid = false;
  _animationActive = false;
  drawWiFiConnectedScreen(
    ssid,
    ip
  );
}

void DisplayManager::switchPage() {

  // If in config mode or selection mode, do not switch normal page
  if (_displayMode == DISPLAY_MODE_CONFIG || wifiManager.isConfigModeActive() || _displayMode == DISPLAY_MODE_SELECTION) {
    return;
  }

  // If in dev mode, cycle to next dev screen
  if (_displayMode == DISPLAY_MODE_DEV) {
    switchTestScreen();
    drawCurrentScreen();
    return;
  }

  // If no battery data is available, do not switch page
  if (!_displayData.hasBattery) {
    return;
  }

  if (
    _currentPage ==
    PAGE_TANK
  ) {

    _currentPage =
      PAGE_BATTERY;

  } else {

    _currentPage =
      PAGE_TANK;
  }

  drawCurrentScreen();
}

// =====================================================
//             UI TIMEOUT & POWER MANAGEMENT
// =====================================================

bool DisplayManager::isAwake() const {
  return _displayAwake;
}

void DisplayManager::wakeDisplay() {
  _displayAwake = true;
  _display.setPowerSave(0);
  _lastUiActivityTime = millis();
  batteryLedManager.setLedsEnabled(true);

  drawCurrentScreen();
}

void DisplayManager::sleepDisplay() {
  _displayAwake = false;
  _display.setPowerSave(1);

  // If not charging, turn OFF all LEDs
  if (!batteryLedManager.isCharging()) {
    batteryLedManager.setLedsEnabled(false);
  }
}

void DisplayManager::resetTimeout() {
  _lastUiActivityTime = millis();
}

// =====================================================
//           CHARGING TRANSITION & ANIMATION
// =====================================================

bool DisplayManager::isChargingAnimationActive() const {
  return _chargingAnimationActive;
}

void DisplayManager::startChargingAnimation() {
  _chargingAnimationActive = true;
  _chargingAnimationStart = millis();
  _savedPageBeforeAnimation = _currentPage;
  _savedTestScreenBeforeAnimation = _currentTestScreen;
  _savedModeBeforeAnimation = _displayMode;

  // Turn OLED ON if it was OFF
  if (!_displayAwake) {
    _display.setPowerSave(0);
    _displayAwake = true;
  }

  // Ensure LEDs are enabled
  batteryLedManager.setLedsEnabled(true);

  drawChargingAnimation();
}

void DisplayManager::checkChargingTransition(bool currentlyCharging) {
  if (!_lastChargingState && currentlyCharging) {
    startChargingAnimation();
  }
  _lastChargingState = currentlyCharging;
}

void DisplayManager::drawChargingAnimation() {
  _display.clearBuffer();

  // 1. Header: "CHARGING"
  _display.setFont(u8g2_font_6x10_tr);
  const char* title = "CHARGING";
  int tw = _display.getStrWidth(title);
  _display.drawStr((_screenW - tw) / 2, 11, title);

  // 2. Center: Battery Outline & Terminal Nub
  int bx = 28;
  int by = 16;
  int bw = 66;
  int bh = 28;
  _display.drawRFrame(bx, by, bw, bh, 3);
  _display.drawRBox(bx + bw, by + 7, 5, bh - 14, 2);

  // 3. Filling Blocks inside battery cell
  unsigned long elapsed = millis() - _chargingAnimationStart;
  int activeBlocks = ((elapsed % 1200) / 240) + 1;
  if (activeBlocks > 4) activeBlocks = 4;

  for (int b = 0; b < activeBlocks; b++) {
    _display.drawBox(bx + 4 + b * 15, by + 4, 12, bh - 8);
  }

  // 4. Footer: Live Voltage and Battery Percentage
  _display.setFont(u8g2_font_5x8_tr);
  char statBuf[32];
  snprintf(statBuf, sizeof(statBuf), "%.2fV  -  %.0f%%", batteryLedManager.getVoltage(), batteryLedManager.getBatteryPercent());
  int sw = _display.getStrWidth(statBuf);
  _display.drawStr((_screenW - sw) / 2, 58, statBuf);

  _display.sendBuffer();
}

// =====================================================
//                 DEV MODE CONTROLLER
// =====================================================

void DisplayManager::setDevMode(bool active) {
  if (active) {
    _displayMode = DISPLAY_MODE_DEV;
    _lastOpenedScreen = DISPLAY_MODE_DEV;
  } else {
    if (_displayMode == DISPLAY_MODE_DEV) {
      _displayMode = DISPLAY_MODE_NORMAL;
    }
    if (_lastOpenedScreen == DISPLAY_MODE_DEV) {
      _lastOpenedScreen = DISPLAY_MODE_NORMAL;
    }
  }
}

bool DisplayManager::isDevMode() const {
  return (_displayMode == DISPLAY_MODE_DEV);
}

// =====================================================
//              SELECTION SCREEN CONTROLLER
// =====================================================

void DisplayManager::openSelectionScreen() {
  _selectionCount = 0;
  _availableOptions[_selectionCount++] = SELECT_OPT_TANK;
  _availableOptions[_selectionCount++] = SELECT_OPT_CONFIG;
  if (devConfig.get().devModeEnabled) {
    _availableOptions[_selectionCount++] = SELECT_OPT_DEV;
  }

  // Pre-select based on current active mode
  if (_displayMode == DISPLAY_MODE_DEV) {
    _selectionIndex = (_selectionCount > 2) ? 2 : 0;
  } else if (_displayMode == DISPLAY_MODE_CONFIG || wifiManager.isConfigModeActive()) {
    _selectionIndex = 1;
  } else {
    _selectionIndex = 0;
  }

  _displayMode = DISPLAY_MODE_SELECTION;
  resetTimeout();
  drawCurrentScreen();
}

void DisplayManager::closeSelectionScreen() {
  if (_lastOpenedScreen == DISPLAY_MODE_DEV && !devConfig.get().devModeEnabled) {
    _lastOpenedScreen = DISPLAY_MODE_NORMAL;
  }
  _displayMode = _lastOpenedScreen;
  resetTimeout();
  drawCurrentScreen();
}

void DisplayManager::nextSelectionItem() {
  if (_displayMode != DISPLAY_MODE_SELECTION || _selectionCount == 0) return;
  _selectionIndex = (_selectionIndex + 1) % _selectionCount;
  resetTimeout();
  drawCurrentScreen();
}

void DisplayManager::confirmSelection() {
  if (_displayMode != DISPLAY_MODE_SELECTION || _selectionCount == 0) return;

  SelectionOption chosen = _availableOptions[_selectionIndex];
  Serial.print("[Display] Screen selection confirmed: ");

  if (chosen == SELECT_OPT_TANK) {
    Serial.println("Tank Data Screen");
    if (wifiManager.isConfigModeActive()) {
      wifiManager.exitConfigMode();
    }
    _displayMode = DISPLAY_MODE_NORMAL;
    _lastOpenedScreen = DISPLAY_MODE_NORMAL;
    _currentPage = PAGE_TANK;
  } else if (chosen == SELECT_OPT_CONFIG) {
    Serial.println("Config Screen");
    _displayMode = DISPLAY_MODE_CONFIG;
    if (!wifiManager.isConfigModeActive()) {
      wifiManager.enterConfigMode();
    }
  } else if (chosen == SELECT_OPT_DEV) {
    Serial.println("Dev Screen");
    if (wifiManager.isConfigModeActive()) {
      wifiManager.exitConfigMode();
    }
    _displayMode = DISPLAY_MODE_DEV;
    _lastOpenedScreen = DISPLAY_MODE_DEV;
  }

  resetTimeout();
  drawCurrentScreen();
}

void DisplayManager::drawSelectionScreen() {
  _display.clearBuffer();

  // 1. Header Bar (inverted black bar with white title)
  _display.drawBox(0, 0, _screenW, 11);
  _display.setDrawColor(0);
  _display.setFont(u8g2_font_6x10_tr);
  const char* title = "SELECT SCREEN";
  int tw = _display.getStrWidth(title);
  _display.drawStr((_screenW - tw) / 2, 9, title);

  // 2. Menu Items
  _display.setFont(u8g2_font_6x10_tr);

  for (uint8_t i = 0; i < _selectionCount; i++) {
    SelectionOption opt = _availableOptions[i];
    const char* label = "Unknown";
    if (opt == SELECT_OPT_TANK) label = "Tank Data Screen";
    else if (opt == SELECT_OPT_CONFIG) label = "Config Screen";
    else if (opt == SELECT_OPT_DEV) label = "Dev Screen";

    int rowY, rowH;
    if (_selectionCount == 2) {
      rowH = 14;
      rowY = 16 + (i * 18);
    } else {
      rowH = 12;
      rowY = 14 + (i * 13);
    }

    if (i == _selectionIndex) {
      // Highlighted selection box
      _display.setDrawColor(1);
      _display.drawRBox(2, rowY, _screenW - 4, rowH, 2);
      _display.setDrawColor(0);
      char itemBuf[28];
      snprintf(itemBuf, sizeof(itemBuf), "> %s", label);
      _display.drawStr(6, rowY + rowH - 3, itemBuf);
    } else {
      // Unselected item
      _display.setDrawColor(1);
      char itemBuf[28];
      snprintf(itemBuf, sizeof(itemBuf), "  %s", label);
      _display.drawStr(6, rowY + rowH - 3, itemBuf);
    }
  }

  // 3. Footer Bar: Navigation Helper
  _display.setDrawColor(1);
  _display.drawHLine(0, 53, _screenW);
  _display.setFont(u8g2_font_4x6_tr);
  const char* footer = "1x: Move   2x: Confirm";
  int fw = _display.getStrWidth(footer);
  _display.drawStr((_screenW - fw) / 2, 61, footer);

  _display.sendBuffer();
}

// =====================================================
//               BUTTON ACTION HANDLERS
// =====================================================

void DisplayManager::handleShortPress() {
  if (!_displayAwake) {
    wakeDisplay();
    return;
  }

  resetTimeout();

  if (_displayMode == DISPLAY_MODE_SELECTION) {
    nextSelectionItem();
  } else if (_displayMode == DISPLAY_MODE_DEV) {
    switchTestScreen();
    drawCurrentScreen();
  } else if (_displayMode == DISPLAY_MODE_CONFIG) {
    // Config mode stays awake
  } else {
    // Normal mode: switch between Tank and Battery pages
    switchPage();
  }
}

void DisplayManager::handleLongPress() {
  if (isChargingAnimationActive()) {
    return;
  }

  if (!_displayAwake) {
    wakeDisplay();
  }

  // If already in selection screen, toggle/close it back to last opened screen
  if (_displayMode == DISPLAY_MODE_SELECTION) {
    closeSelectionScreen();
    return;
  }

  openSelectionScreen();
}

void DisplayManager::switchTestScreen() {
  if (TEST_SCREEN_COUNT > 0) {
    _currentTestScreen = (TestScreen)((_currentTestScreen + 1) % TEST_SCREEN_COUNT);
    Serial.print("Test screen navigated to: ");
    Serial.println((int)_currentTestScreen);
  }
}

void DisplayManager::updateTestScreen() {
  updateTestScreen(batteryLedManager);
}

void DisplayManager::updateTestScreen(const BatteryLedManager& batteryLed) {
  if (!_displayAwake || _chargingAnimationActive) {
    return;
  }
  switch (_currentTestScreen) {
    case TEST_SCREEN_INA219:
    default:
      drawIna219TestScreen(batteryLed);
      break;
  }
}

void DisplayManager::update() {
  unsigned long now = millis();

  // 1. Handle Charging Animation if running
  if (_chargingAnimationActive) {
    if (now - _chargingAnimationStart >= CHARGING_ANIMATION_DURATION_MS) {
      _chargingAnimationActive = false;
      // Restore previously active page & mode
      _currentPage = _savedPageBeforeAnimation;
      _currentTestScreen = _savedTestScreenBeforeAnimation;
      _displayMode = _savedModeBeforeAnimation;
      // Start fresh 15-second display timeout
      _lastUiActivityTime = millis();
      // Redraw restored page
      drawCurrentScreen();
    } else {
      // Redraw animation frame at ~30 FPS (every 33ms)
      static unsigned long lastAnimFrame = 0;
      if (now - lastAnimFrame >= 33) {
        lastAnimFrame = now;
        drawChargingAnimation();
      }
    }
    return;
  }

  // Handle Configuration Mode rendering & transitions
  bool isConfig = wifiManager.isConfigModeActive();
  if (isConfig != _wasConfigModeActive) {
    _wasConfigModeActive = isConfig;
    _lastUiActivityTime = now; // Start fresh 15s timer from the moment config mode exits
    if (!_displayAwake) {
      wakeDisplay();
    }

    if (!isConfig) {
      // Configuration mode exited or timed out (auto-dismissed)
      // Edge case: config mode auto dismiss will always point to the last opened screen.
      // If the last opened screen is not present, fallback to tank data screens.
      if (_lastOpenedScreen == DISPLAY_MODE_DEV) {
        if (devConfig.get().devModeEnabled) {
          _displayMode = DISPLAY_MODE_DEV;
        } else {
          _displayMode = DISPLAY_MODE_NORMAL;
          _lastOpenedScreen = DISPLAY_MODE_NORMAL;
        }
      } else {
        _displayMode = DISPLAY_MODE_NORMAL;
      }
    } else {
      _displayMode = DISPLAY_MODE_CONFIG;
    }

    drawCurrentScreen();
  }

  if (isConfig) {
    // Keep display awake while in Configuration Mode
    _lastUiActivityTime = now;
    if (!_displayAwake) {
      wakeDisplay();
    }

    static unsigned long lastConfigScreenUpdate = 0;
    if (now - lastConfigScreenUpdate >= 1000) {
      lastConfigScreenUpdate = now;
      drawCurrentScreen();
    }
    return;
  }

  // 2. Handle UI Inactivity Timeout (Normal Mode)
  _uiTimeoutMs = systemConfig.get().uiTimeoutMs;
  if (_displayAwake) {
    if (systemConfig.get().autoSleepEnabled && _uiTimeoutMs > 0) {
      if (now - _lastUiActivityTime >= _uiTimeoutMs) {
        if (_displayMode == DISPLAY_MODE_SELECTION) {
          if (_lastOpenedScreen == DISPLAY_MODE_DEV && !devConfig.get().devModeEnabled) {
            _lastOpenedScreen = DISPLAY_MODE_NORMAL;
          }
          _displayMode = _lastOpenedScreen;
        }
        sleepDisplay();
      }
    }
  }

  // If display is asleep, do not render screens
  if (!_displayAwake) {
    return;
  }

  // 3. Screen Rendering
  if (_displayMode == DISPLAY_MODE_DEV) {
    // In Dev Mode: update live test screen at ~30 FPS (every 33ms)
    static unsigned long lastDevScreenUpdate = 0;
    if (now - lastDevScreenUpdate >= 33) {
      lastDevScreenUpdate = now;
      drawCurrentScreen();
    }
    return;
  }

  // Normal mode screen rendering / animations
  updateAnimation();

  if (!_displayData.valid) {
    // Animate hourglass sand timer when waiting for transmitter
    if (now - _lastWaitAnimUpdate >= WAIT_ANIM_INTERVAL_MS) {
      _lastWaitAnimUpdate = now;
      _waitAnimAngle += 5.0f;
      if (_waitAnimAngle >= 360.0f) {
        _waitAnimAngle -= 360.0f;
      }
      drawNotConnectedScreen();
    }
  } else if (_animationActive) {
    drawCurrentScreen();
  } else if (now - _lastFooterUpdate >= 1000) {
    // Refresh live "updated ... ago" footer every second
    _lastFooterUpdate = now;
    drawCurrentScreen();
  }
}

// =====================================================
//             INA219 REALTIME TEST SCREEN
// =====================================================

void DisplayManager::drawIna219TestScreen(const BatteryLedManager& batteryLed) {
  drawIna219TestScreen(
    batteryLed.getVoltage(),
    batteryLed.getShuntVoltage_mV(),
    batteryLed.getLoadVoltage_V(),
    batteryLed.getCurrent_mA(),
    batteryLed.getPower_mW(),
    batteryLed.getBatteryPercent(),
    batteryLed.getChargingStateStr(),
    batteryLed.isSensorConnected(),
    batteryLed.getLedPinState(0),
    batteryLed.getLedPinState(1),
    batteryLed.getLedPinState(2),
    batteryLed.getLedPinState(3),
    batteryLed.getLedPinState(4)
  );
}

void DisplayManager::drawIna219TestScreen(
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
) {
  _display.clearBuffer();

  // Header bar (inverted box with title & sensor status)
  _display.drawBox(0, 0, _screenW, 11);
  _display.setDrawColor(0);
  _display.setFont(u8g2_font_6x10_tr);

  char titleBuf[24];
  if (TEST_SCREEN_COUNT > 1) {
    snprintf(titleBuf, sizeof(titleBuf), "DEV INA219 [%d/%d]", (int)_currentTestScreen + 1, (int)TEST_SCREEN_COUNT);
  } else {
    snprintf(titleBuf, sizeof(titleBuf), "DEV: INA219");
  }
  _display.drawStr(3, 9, titleBuf);

  if (isConnected) {
    _display.drawStr(_screenW - 16, 9, "OK");
  } else {
    _display.drawStr(_screenW - 22, 9, "ERR");
  }

  // Body: Real-time sensor metrics
  _display.setDrawColor(1);
  _display.setFont(u8g2_font_5x8_tr);

  // Line 1: VBUS and BATT %
  char line1[32];
  snprintf(line1, sizeof(line1), "VBUS: %.3fV  BAT: %.0f%%", busV, batPct);
  _display.drawStr(2, 20, line1);

  // Line 2: CURRENT and POWER
  char line2[32];
  snprintf(line2, sizeof(line2), "CUR: %.1fmA PWR: %.0fmW", currentMa, powerMw);
  _display.drawStr(2, 29, line2);

  // Line 3: VSHUNT and VLOAD
  char line3[32];
  snprintf(line3, sizeof(line3), "VSH: %.1fmV  VLD: %.2fV", shuntMv, loadV);
  _display.drawStr(2, 38, line3);

  // Line 4: OPERATING STATE
  char line4[32];
  snprintf(line4, sizeof(line4), "STATE: %s", stateStr);
  _display.drawStr(2, 47, line4);

  // Divider line
  _display.drawHLine(0, 50, _screenW);

  // Footer: 5-LED Live Visual Indicator
  _display.drawStr(2, 60, "LEDS:");

  bool leds[5] = { led1, led2, led3, led4, led5 };
  for (int i = 0; i < 5; i++) {
    int cx = 38 + i * 18;
    int cy = 57;
    if (leds[i]) {
      _display.drawDisc(cx, cy, 3);
    } else {
      _display.drawCircle(cx, cy, 3);
    }
    char numStr[2];
    numStr[0] = '1' + i;
    numStr[1] = '\0';
    _display.drawStr(cx + 5, 60, numStr);
  }

  _display.sendBuffer();
}

// =====================================================
//             DEDICATED CONFIGURATION SCREEN
// =====================================================

void DisplayManager::drawConfigScreen(
  const String& ssid,
  const String& url,
  const String& ip,
  unsigned long remainingSec
) {
  _display.clearBuffer();

  // Header: inverted banner
  _display.drawBox(0, 0, _screenW, 11);
  _display.setDrawColor(0);
  _display.setFont(u8g2_font_6x10_tr);
  const char* title = "CONFIG MODE";
  int titleWidth = _display.getStrWidth(title);
  _display.drawStr((_screenW - titleWidth) / 2, 9, title);

  // Body
  _display.setDrawColor(1);
  setSmallFont();
  String netLine = "Wi-Fi: " + (ssid.length() > 0 ? ssid : String("DASSHOME-Setup"));
  _display.drawStr(2, 22, netLine.c_str());

  String urlLine = "URL:   " + url;
  _display.drawStr(2, 33, urlLine.c_str());

  String ipLine = "IP:    " + ip;
  _display.drawStr(2, 44, ipLine.c_str());

  // Divider line
  _display.drawHLine(0, 48, _screenW);

  // Footer: Countdown timer / hold button to exit
  char footBuf[32];
  snprintf(footBuf, sizeof(footBuf), "Auto-exit: %lum %02lus", remainingSec / 60, remainingSec % 60);
  int footWidth = _display.getStrWidth(footBuf);
  _display.drawStr((_screenW - footWidth) / 2, 59, footBuf);

  _display.sendBuffer();
}
