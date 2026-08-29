#include "LoRaManager.h"

// Instantiate global LoRaManager
LoRaManager loraManager;

LoRaManager::LoRaManager()
  : _loraSerial(2) {
}

// =====================================================
//                    LORA COMMAND
// =====================================================

String LoRaManager::command(
  const char* cmd,
  unsigned long waitTime
) {

  while (
    _loraSerial.available()
  ) {

    _loraSerial.read();
  }

  Serial.print(
    "LoRa >> "
  );

  Serial.println(
    cmd
  );

  _loraSerial.println(
    cmd
  );

  String response;

  unsigned long start =
    millis();

  while (
    millis() -
    start <
    waitTime
  ) {

    while (
      _loraSerial.available()
    ) {

      char c =
        _loraSerial.read();

      response += c;
    }

    // Exit immediately once RYLR998 responds with +OK, +ERR, or line termination
    if (
      response.indexOf("+OK") >= 0 ||
      response.indexOf("+ERR") >= 0 ||
      (response.length() > 0 && response.endsWith("\n"))
    ) {
      break;
    }

    // Keep display animations rendering smoothly during startup
    displayManager.update();
    delay(2);
  }

  response.trim();

  Serial.print(
    "LoRa << "
  );

  Serial.println(
    response
  );

  return response;
}

// =====================================================
//                 LORA INITIALIZATION
// =====================================================

void LoRaManager::begin() {

  _loraSerial.begin(
    loraConfig.baudRate,
    SERIAL_8N1,
    LORA_RX,
    LORA_TX
  );

  // Quick initial delay while keeping display alive
  unsigned long start = millis();
  while (millis() - start < 100) {
    displayManager.update();
    delay(5);
  }

  command(
    "AT",
    250
  );

  // ---------------------------------------------------
  // Receiver address
  // ---------------------------------------------------

  String cmd =
    "AT+ADDRESS=" +
    String(
      loraConfig.address
    );

  command(
    cmd.c_str()
  );

  // ---------------------------------------------------
  // Network ID
  // ---------------------------------------------------

  cmd =
    "AT+NETWORKID=" +
    String(
      loraConfig.networkId
    );

  command(
    cmd.c_str()
  );

  // ---------------------------------------------------
  // Band
  // ---------------------------------------------------

  cmd =
    "AT+BAND=" +
    String(
      loraConfig.band
    );

  command(
    cmd.c_str()
  );

  // ---------------------------------------------------
  // RF parameters
  // ---------------------------------------------------

  cmd =
    "AT+PARAMETER=" +
    String(
      loraConfig.spreadingFactor
    ) +
    "," +
    String(
      loraConfig.bandwidth
    ) +
    "," +
    String(
      loraConfig.codingRate
    ) +
    "," +
    String(
      loraConfig.preambleLength
    );

  command(
    cmd.c_str()
  );

  // ===================================================
  // Verify
  // ===================================================

  Serial.println();
  Serial.println(
    "========== LoRa Config =========="
  );

  command(
    "AT+ADDRESS?"
  );

  command(
    "AT+NETWORKID?"
  );

  command(
    "AT+BAND?"
  );

  command(
    "AT+PARAMETER?"
  );

  Serial.println(
    "=================================="
  );
}

// =====================================================
//               RYLR998 RECEIVE PARSER
// =====================================================

bool LoRaManager::parseLoRaReceive(
  const String &line,
  RawTelemetry &raw,
  int &rssi,
  int &snr
) {

  if (
    !line.startsWith(
      "+RCV="
    )
  ) {

    return false;
  }

  String payload =
    line.substring(
      5
    );

  int comma1 =
    payload.indexOf(',');

  if (
    comma1 < 0
  ) {

    return false;
  }

  int comma2 =
    payload.indexOf(
      ',',
      comma1 + 1
    );

  if (
    comma2 < 0
  ) {

    return false;
  }

  int comma3 =
    payload.indexOf(
      ',',
      comma2 + 1
    );

  if (
    comma3 < 0
  ) {

    return false;
  }

  int comma4 =
    payload.indexOf(
      ',',
      comma3 + 1
    );

  if (
    comma4 < 0
  ) {

    return false;
  }

  // ---------------------------------------------------
  // Native LoRa fields
  // ---------------------------------------------------

  int sourceAddress =
    payload.substring(
      0,
      comma1
    ).toInt();

  int packetLength =
    payload.substring(
      comma1 + 1,
      comma2
    ).toInt();

  String applicationData =
    payload.substring(
      comma2 + 1,
      comma3
    );

  rssi =
    payload.substring(
      comma3 + 1,
      comma4
    ).toInt();

  snr =
    payload.substring(
      comma4 + 1
    ).toInt();

  // ---------------------------------------------------
  // Basic validation
  // ---------------------------------------------------

  if (
    sourceAddress < 0 ||
    packetLength <= 0 ||
    applicationData.length() == 0
  ) {

    Serial.println(
      "Invalid LoRa envelope"
    );

    return false;
  }

  // ---------------------------------------------------
  // Parse DASS HOME application data
  // ---------------------------------------------------

  ParsedPacket parsed;

  if (
    !parseApplicationPacket(
      applicationData,
      sourceAddress,
      parsed
    )
  ) {

    return false;
  }

  // ---------------------------------------------------
  // Build raw telemetry
  // ---------------------------------------------------

  raw.transmitterId =
    sourceAddress;

  raw.sequence =
    parsed.sequence;

  raw.distanceCm =
    parsed.distanceCm;

  raw.batteryVoltage =
    parsed.batteryVoltage;

  raw.charging =
    parsed.charging;

  raw.hasBattery =
    parsed.batteryFound;

  // ---------------------------------------------------
  // Debug
  // ---------------------------------------------------

  Serial.println();
  Serial.println(
    "========== LoRa RX =========="
  );

  Serial.print(
    "Transmitter: "
  );

  Serial.println(
    raw.transmitterId
  );

  Serial.print(
    "Length: "
  );

  Serial.println(
    packetLength
  );

  Serial.print(
    "Sequence: "
  );

  Serial.println(
    raw.sequence
  );

  Serial.print(
    "Distance: "
  );

  Serial.print(
    raw.distanceCm,
    1
  );

  Serial.println(
    " cm"
  );

  Serial.print(
    "Battery: "
  );

  Serial.print(
    raw.batteryVoltage,
    2
  );

  Serial.println(
    " V"
  );

  Serial.print(
    "Charging: "
  );

  Serial.println(
    raw.charging
      ? "YES"
      : "NO"
  );

  Serial.print(
    "RSSI: "
  );

  Serial.println(
    rssi
  );

  Serial.print(
    "SNR: "
  );

  Serial.println(
    snr
  );

  Serial.println(
    "============================="
  );

  return true;
}

// =====================================================
//                  RECEIVE FROM LORA
// =====================================================

bool LoRaManager::receive(
  RawTelemetry &raw,
  int &rssi,
  int &snr
) {

  bool received =
    false;

  while (
    _loraSerial.available()
  ) {

    String line =
      _loraSerial.readStringUntil(
        '\n'
      );

    line.trim();

    if (
      line.length() == 0
    ) {

      continue;
    }

    if (
      line.startsWith(
        "+RCV="
      )
    ) {

      if (
        parseLoRaReceive(
          line,
          raw,
          rssi,
          snr
        )
      ) {

        received =
          true;
      }
    }

    else {

      // Other RYLR998 responses are ignored.
      Serial.print(
        "LoRa: "
      );

      Serial.println(
        line
      );
    }
  }

  return received;
}
