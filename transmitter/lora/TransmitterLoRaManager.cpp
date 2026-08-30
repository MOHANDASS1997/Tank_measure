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

  // Build payload: TS|v=1|seq=123|dist=43.2|bat=3.87|chg=0
  String payload = String(TX_PACKET_HEADER) +
                   "|v=" + String(TX_PACKET_VERSION) +
                   "|seq=" + String(sequence) +
                   "|dist=" + String(distanceCm, 1) +
                   "|bat=" + String(batteryVoltage, 2) +
                   "|chg=" + String(charging ? 1 : 0);

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
