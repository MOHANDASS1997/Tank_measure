#include "DisplayManager.h"

// Instantiate global DisplayManager
DisplayManager displayManager;

DisplayManager::DisplayManager()
  : _display(U8G2_R0, U8X8_PIN_NONE),
    _currentPage(PAGE_TANK),
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
  drawCurrentScreen();
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

void DisplayManager::update() {

  updateAnimation();

  if (!_displayData.valid) {
    // Animate hourglass sand timer when waiting for transmitter
    if (millis() - _lastWaitAnimUpdate >= WAIT_ANIM_INTERVAL_MS) {
      _lastWaitAnimUpdate = millis();
      _waitAnimAngle += 5.0f;
      if (_waitAnimAngle >= 360.0f) {
        _waitAnimAngle -= 360.0f;
      }
      drawNotConnectedScreen();
    }
  } else if (_animationActive) {
    drawCurrentScreen();
  } else if (millis() - _lastFooterUpdate >= 1000) {
    // Refresh live "updated ... ago" footer every second
    _lastFooterUpdate = millis();
    drawCurrentScreen();
  }
}
