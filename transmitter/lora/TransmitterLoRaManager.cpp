#include "TransmitterLoRaManager.h"

// Instantiate global TransmitterLoRaManager
TransmitterLoRaManager transmitterLoRaManager;

TransmitterLoRaManager::TransmitterLoRaManager() :
  _loraSerial(1) {
}

void TransmitterLoRaManager::begin() {
  pinMode(LORA_RX_PIN, INPUT_PULLUP);
  _loraSerial.begin(
    txConfig.baudRate,
    SERIAL_8N1,
    LORA_RX_PIN,
    LORA_TX_PIN
  );
  delay(100);
}

bool TransmitterLoRaManager::sendAT(const String& cmd, unsigned long timeoutMs) {
  Serial.print("LoRa TX >>> ");
  Serial.println(cmd);

  // Clear incoming buffer
  while (_loraSerial.available()) {
    _loraSerial.read();
  }

  _loraSerial.print(cmd);
  _loraSerial.print("\r\n");

  unsigned long start = millis();
  String response = "";

  while (millis() - start < timeoutMs) {
    while (_loraSerial.available()) {
      char c = (char)_loraSerial.read();
      // Only append valid printable ASCII characters, carriage return, and newline
      // This filters out RF EMI framing noise / null bytes that break String::indexOf
      if ((c >= 32 && c <= 126) || c == '\r' || c == '\n') {
        response += c;
      }
    }
    if (response.indexOf("+OK") >= 0 || response.indexOf("+ERR") >= 0) {
      break;
    }
    delay(5);
  }

  response.trim();
  Serial.print("LoRa RX <<< ");
  Serial.println(response);

  return (response.indexOf("+OK") >= 0);
}

bool TransmitterLoRaManager::configure() {
  Serial.println("Configuring RYLR998 LoRa module...");

  sendAT("AT", 500);

  String addrCmd = "AT+ADDRESS=" + String(txConfig.address);
  sendAT(addrCmd, 500);

  String netCmd = "AT+NETWORKID=" + String(txConfig.networkId);
  sendAT(netCmd, 500);

  String bandCmd = "AT+BAND=" + String(txConfig.band);
  sendAT(bandCmd, 500);

  String paramCmd = "AT+PARAMETER=" +
                    String(txConfig.spreadingFactor) + "," +
                    String(txConfig.bandwidth) + "," +
                    String(txConfig.codingRate) + "," +
                    String(txConfig.preambleLength);
  sendAT(paramCmd, 500);

  String pwrCmd = "AT+CRFOP=" + String(txConfig.outputPower);
  sendAT(pwrCmd, 500);

  Serial.println("RYLR998 configuration completed.");
  return true;
}

bool TransmitterLoRaManager::sendTelemetry(
  unsigned long sequence,
  float distanceCm,
  float batteryVoltage,
  bool charging
) {

  // Build payload: TS|v=1|seq=123|dist=43.2|bat=3.87|chg=0|wds=60|sps=5|sim=30
  const TxOpSettings& op = txOpConfig.get();
  String payload = String(TX_PACKET_HEADER) +
                   "|v=" + String(TX_PACKET_VERSION) +
                   "|seq=" + String(sequence) +
                   "|dist=" + String(distanceCm, 1) +
                   "|bat=" + String(batteryVoltage, 2) +
                   "|chg=" + String(charging ? 1 : 0) +
                   "|wds=" + String(op.wakeDurationSec) +
                   "|sps=" + String(op.samplesPerWake) +
                   "|sim=" + String(op.samplingIntervalMs);

  Serial.println("==================================================");
  Serial.println("Broadcasting Telemetry Packet:");
  Serial.println(payload);
  Serial.println("==================================================");

  String sendCmd = "AT+SEND=" +
                   String(txConfig.targetReceiver) + "," +
                   String(payload.length()) + "," +
                   payload;

  bool success = sendAT(sendCmd, 1500);

  // If transmission failed or module reset (+READY), re-apply configuration
  if (!success) {
    Serial.println("Warning: Transmission failed or module reset. Re-configuring module...");
    configure();
  }

  return success;
}

bool TransmitterLoRaManager::listenForConfigResponse(unsigned long timeoutMs) {
  Serial.println("[LoRa] Listening for receiver response...");

  unsigned long start = millis();
  String buffer = "";

  while (millis() - start < timeoutMs) {
    while (_loraSerial.available()) {
      char c = (char)_loraSerial.read();
      // Filter noise — same as sendAT()
      if ((c >= 32 && c <= 126) || c == '\r' || c == '\n') {
        buffer += c;
      }
    }

    // ---------------------------------------------------
    // Check for a +RCV= envelope (receiver sent something)
    // Exit early as soon as any reply arrives — this is the
    // battery-friendly path: avoids the full 2-second idle.
    // ---------------------------------------------------
    int rcvIdx = buffer.indexOf("+RCV=");
    if (rcvIdx >= 0) {
      Serial.println("[LoRa] Response received.");

      // Extract the application data from the +RCV= envelope:
      // +RCV=<addr>,<len>,<data>,<rssi>,<snr>
      String envelope = buffer.substring(rcvIdx + 5); // skip "+RCV="
      int c1 = envelope.indexOf(',');
      int c2 = (c1 >= 0) ? envelope.indexOf(',', c1 + 1) : -1;
      int c3 = (c2 >= 0) ? envelope.indexOf(',', c2 + 1) : -1;

      if (c1 >= 0 && c2 >= 0 && c3 >= 0) {
        String appData = envelope.substring(c2 + 1, c3);
        appData.trim();

        // ---------------------------------------------------
        // SET_CONFIG handling
        // Format: "SET_CONFIG\n<wds>\n<sps>\n<sim>"
        // ---------------------------------------------------
        if (appData.startsWith("SET_CONFIG")) {
          Serial.println("[LoRa] SET_CONFIG received. Parsing...");

          // Split on newline
          int nl1 = appData.indexOf('\n');
          int nl2 = (nl1 >= 0) ? appData.indexOf('\n', nl1 + 1) : -1;
          int nl3 = (nl2 >= 0) ? appData.indexOf('\n', nl2 + 1) : -1;

          if (nl1 >= 0 && nl2 >= 0 && nl3 >= 0) {
            TxOpSettings newCfg;
            newCfg.wakeDurationSec    = (uint16_t)appData.substring(nl1 + 1, nl2).toInt();
            newCfg.samplesPerWake     = (uint8_t) appData.substring(nl2 + 1, nl3).toInt();
            newCfg.samplingIntervalMs = (uint16_t)appData.substring(nl3 + 1).toInt();

            String err;
            if (txOpConfig.validate(newCfg, err)) {
              txOpConfig.set(newCfg);
              txOpConfig.save();
              Serial.printf("[LoRa] Config updated: wds=%u s, sps=%u, sim=%u ms\n",
                            newCfg.wakeDurationSec,
                            newCfg.samplesPerWake,
                            newCfg.samplingIntervalMs);
            } else {
              Serial.print("[LoRa] SET_CONFIG rejected (invalid values): ");
              Serial.println(err);
            }
          } else {
            Serial.println("[LoRa] SET_CONFIG malformed — missing newline fields.");
          }
        } else {
          // Normal ACK — no config change needed
          Serial.println("[LoRa] ACK received. Config in sync.");
        }
      }

      return true; // Response handled; exit early
    }

    delay(5);
  }

  Serial.println("[LoRa] No response within timeout. Continuing with existing config.");
  return false;
}
