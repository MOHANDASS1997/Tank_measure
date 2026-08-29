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
    _animationActive(false) {

  _displayData.valid = false;
  _displayData.transmitterId = 0;
  _displayData.tankId = 0;
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
//                    PAGE INDICATOR
// =====================================================

void DisplayManager::drawPageIndicator() {

  int radius =
    max(
      2,
      (int)round(
        _minDim * 0.035
      )
    );

  int gap =
    radius * 4;

  int centerX =
    _screenW / 2;

  int y =
    _screenH -
    radius -
    max(
      1,
      (int)round(
        _minDim * 0.02
      )
    );

  int leftX =
    centerX -
    gap / 2;

  int rightX =
    centerX +
    gap / 2;

  if (
    _currentPage ==
    PAGE_TANK
  ) {

    _display.drawDisc(
      leftX,
      y,
      radius
    );

    _display.drawCircle(
      rightX,
      y,
      radius
    );

  } else {

    _display.drawCircle(
      leftX,
      y,
      radius
    );

    _display.drawDisc(
      rightX,
      y,
      radius
    );
  }
}

// =====================================================
//                 NOT CONNECTED SCREEN
// =====================================================

void DisplayManager::drawNotConnectedScreen() {

  _display.clearBuffer();

  setLabelFont();

  const char* title =
    "TANK_SYNC";

  int titleWidth =
    _display.getStrWidth(
      title
    );

  drawTextTop(
    (
      _screenW -
      titleWidth
    ) / 2,
    (int)round(
      _screenH * 0.22
    ),
    title
  );

  setLabelFont();

  const char* message =
    "Not connected";

  int messageWidth =
    _display.getStrWidth(
      message
    );

  int messageHeight =
    _display.getAscent() -
    _display.getDescent();

  int messageTop =
    (
      _screenH -
      messageHeight
    ) / 2;

  drawTextTop(
    (
      _screenW -
      messageWidth
    ) / 2,
    messageTop,
    message
  );

  setSmallFont();

  const char* waiting =
    "Waiting for LoRa...";

  int waitingWidth =
    _display.getStrWidth(
      waiting
    );

  drawTextTop(
    (
      _screenW -
      waitingWidth
    ) / 2,
    (int)round(
      _screenH * 0.68
    ),
    waiting
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
      7,
      (int)round(
        _screenH * 0.10
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
    max(
      2,
      (int)round(
        _screenH * 0.025
      )
    );

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
    max(
      2,
      (int)round(
        _screenH * 0.045
      )
    );

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
    "L/" +
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
    percentageHeight +
    max(
      3,
      (int)round(
        _screenH * 0.035
      )
    );

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

  drawPageIndicator();

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
      7,
      (int)round(
        _screenH * 0.10
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
    max(
      2,
      (int)round(
        _screenH * 0.025
      )
    );

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

  drawPageIndicator();

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
  startAnimation();
  drawCurrentScreen();
}

void DisplayManager::showNotConnected() {

  _displayData.valid = false;
  _animationActive = false;
  drawNotConnectedScreen();
}

void DisplayManager::switchPage() {

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

  if (
    _animationActive
  ) {

    drawCurrentScreen();
  }

  // ===================================================
  // Connection timeout
  // ===================================================

  if (
    _displayData.valid &&
    millis() -
      _displayData.lastReceived >
      LORA_TIMEOUT_MS
  ) {

    _displayData.valid =
      false;

    _animationActive =
      false;

    Serial.println(
      "LoRa timeout - disconnected"
    );

    drawNotConnectedScreen();
  }
}
