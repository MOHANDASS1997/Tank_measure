#include "LoRaManager.h"
#include "../time/TimeManager.h"
#include "../config/TimeConfig.h"

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

  _loraSerial.print(
    cmd
  );
  _loraSerial.print(
    "\r\n"
  );

  String response = "";

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

      if ((c >= 32 && c <= 126) || c == '\r' || c == '\n') {
        response += c;
      }
    }

    // Exit immediately once RYLR998 responds with +OK, +ERR, or line termination
    if (
      response.indexOf("+OK") >= 0 ||
      response.indexOf("+ERR") >= 0 ||
      (response.length() > 0 && response.endsWith("\n"))
    ) {
      break;
    }

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

  pinMode(
    LORA_RX,
    INPUT_PULLUP
  );

  _loraSerial.begin(
    loraConfig.baudRate,
    SERIAL_8N1,
    LORA_RX,
    LORA_TX
  );

  delay(100);

  Serial.println("Configuring RYLR998 Receiver...");

  command("AT", 500);
  delay(50);

  // ---------------------------------------------------
  // Receiver address
  // ---------------------------------------------------
  String cmd = "AT+ADDRESS=" + String(loraConfig.address);
  command(cmd.c_str(), 500);
  delay(50);

  // ---------------------------------------------------
  // Network ID
  // ---------------------------------------------------
  cmd = "AT+NETWORKID=" + String(loraConfig.networkId);
  command(cmd.c_str(), 500);
  delay(50);

  // ---------------------------------------------------
  // Band
  // ---------------------------------------------------
  cmd = "AT+BAND=" + String(loraConfig.band);
  command(cmd.c_str(), 500);
  delay(50);

  // ---------------------------------------------------
  // RF parameters
  // ---------------------------------------------------
  cmd = "AT+PARAMETER=" +
        String(loraConfig.spreadingFactor) + "," +
        String(loraConfig.bandwidth) + "," +
        String(loraConfig.codingRate) + "," +
        String(loraConfig.preambleLength);
  command(cmd.c_str(), 500);
  delay(50);

  // ===================================================
  // Verify
  // ===================================================
  Serial.println();
  Serial.println("========== LoRa Config ==========");
  command("AT+ADDRESS?", 500);
  delay(50);
  command("AT+NETWORKID?", 500);
  delay(50);
  command("AT+BAND?", 500);
  delay(50);
  command("AT+PARAMETER?", 500);
  Serial.println("==================================");
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

  bool packetValid = parseApplicationPacket(
    applicationData,
    sourceAddress,
    parsed
  );

  // ---------------------------------------------------
  // Reply to transmitter: SET_CONFIG (mismatch) or +ACK (in-sync / fallback)
  //
  // MUST ALWAYS RUN — even if packetValid is false!
  // The acknowledgement payload is independent of whether the
  // packet telemetry was accepted or rejected.
  // If there is a mismatch between the reported sleep duration / sampling
  // and the calculated values, send SET_CONFIG with the correct values.
  // Otherwise, send pure +ACK.
  // ---------------------------------------------------

  TransmitterConfig txCfg;
  bool cfgFound = transmitterConfig.findTransmitter(sourceAddress, txCfg);

  if (cfgFound && parsed.wdsFound && parsed.spsFound && parsed.simFound) {
    // Resolve the correct wake duration for the current local time
    uint8_t localHour = 0;
    uint32_t epoch = timeManager.getEpoch();
    if (epoch > 0) {
      long offsetSec = timeConfig.get().gmtOffsetSec;
      localHour = (uint8_t)(((epoch + (uint32_t)offsetSec) % 86400UL) / 3600UL);
    }
    uint16_t expectedWakeSec = transmitterConfig.getEffectiveWakeSec(sourceAddress, localHour);

    bool mismatch =
      (parsed.wakeDurationSec    != expectedWakeSec)           ||
      (parsed.samplesPerWake     != txCfg.samplesPerWake)      ||
      (parsed.samplingIntervalMs != txCfg.samplingIntervalMs);

    if (mismatch) {
      Serial.printf("[LoRa] Config mismatch for %d (packetValid=%d): reported wds=%u sps=%u sim=%u / "
                    "expected wds=%u sps=%u sim=%u. Sending SET_CONFIG.\n",
                    sourceAddress, (int)packetValid,
                    parsed.wakeDurationSec, parsed.samplesPerWake, parsed.samplingIntervalMs,
                    expectedWakeSec, txCfg.samplesPerWake, txCfg.samplingIntervalMs);

      String msg = "SET_CONFIG\n" +
                   String(expectedWakeSec)          + "\n" +
                   String(txCfg.samplesPerWake)     + "\n" +
                   String(txCfg.samplingIntervalMs);

      String sendCmd = "AT+SEND=" + String(sourceAddress) + "," +
                       String(msg.length()) + "," + msg;
      command(sendCmd.c_str(), 500);

    } else {
      // All values already in sync — send pure +ACK (no config payload)
      Serial.println("[LoRa] Config in sync. Sending ACK.");
      String sendCmd = "AT+SEND=" + String(sourceAddress) + ",4,+ACK";
      command(sendCmd.c_str(), 500);
    }

  } else {
    // Transmitter not in config, or packet predates op-config fields.
    // Always ACK so the transmitter can exit early.
    if (!cfgFound) {
      Serial.printf("[LoRa] Transmitter %d not in config. Sending ACK.\n", sourceAddress);
    } else {
      Serial.println("[LoRa] Packet missing op-config fields. Sending ACK.");
    }
    String sendCmd = "AT+SEND=" + String(sourceAddress) + ",4,+ACK";
    command(sendCmd.c_str(), 500);
  }

  // ---------------------------------------------------
  // If packet failed validation, do NOT forward telemetry
  // ---------------------------------------------------
  if (!packetValid) {
    Serial.println("[LoRa] Packet rejected due to invalid/null/negative payload. (ACK/SET_CONFIG sent).");
    return false;
  }

  // ---------------------------------------------------
  // Build raw telemetry (only for valid packets)
  // ---------------------------------------------------

  raw.transmitterAddress =
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

  raw.hasOpConfig =
    (parsed.wdsFound && parsed.spsFound && parsed.simFound);

  raw.wakeDurationSec =
    parsed.wakeDurationSec;

  raw.samplesPerWake =
    parsed.samplesPerWake;

  raw.samplingIntervalMs =
    parsed.samplingIntervalMs;

  // ---------------------------------------------------
  // Debug
  // ---------------------------------------------------

  Serial.println();
  Serial.println(
    "========== LoRa RX =========="
  );

  Serial.print(
    "Transmitter Address: "
  );

  Serial.println(
    raw.transmitterAddress
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

    String rawLine =
      _loraSerial.readStringUntil(
        '\n'
      );

    // Filter out RF EMI framing noise and non-printable bytes
    String line = "";
    for (size_t i = 0; i < rawLine.length(); i++) {
      char c = rawLine[i];
      if ((c >= 32 && c <= 126) || c == '\r') {
        line += c;
      }
    }
    line.trim();

    if (
      line.length() == 0
    ) {

      continue;
    }

    int rcvIndex =
      line.indexOf("+RCV=");

    if (
      rcvIndex >= 0
    ) {

      String cleanRCV =
        line.substring(rcvIndex);

      if (
        parseLoRaReceive(
          cleanRCV,
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

      // Only print debug if line contains meaningful text
      bool hasReadable = false;
      for (size_t i = 0; i < line.length(); i++) {
        if (isAlphaNumeric(line[i])) {
          hasReadable = true;
          break;
        }
      }

      if (hasReadable) {
        Serial.print(
          "LoRa: "
        );

        Serial.println(
          line
        );
      }
    }
  }

  return received;
}
