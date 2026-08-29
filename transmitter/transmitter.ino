#define TRIG_PIN 4
#define ECHO_PIN 3

#define LORA_RX_PIN 6   // ESP32 receives from RYLR998 TXD
#define LORA_TX_PIN 7   // ESP32 transmits to RYLR998 RXD

HardwareSerial LoRaSerial(1);

void sendAT(const char* command) {
  Serial.print(">>> ");
  Serial.println(command);

  LoRaSerial.print(command);
  LoRaSerial.print("\r\n");

  unsigned long start = millis();

  while (millis() - start < 1000) {
    while (LoRaSerial.available()) {
      Serial.write(LoRaSerial.read());
    }
  }

  Serial.println();
  Serial.println("------------------------------");
}

void setup() {
  Serial.begin(115200);

  // ==========================================
  // ULTRASONIC SENSOR — KEEP AS IS
  // ==========================================

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(TRIG_PIN, LOW);


  // ==========================================
  // RYLR998 — NEW
  // ==========================================

  LoRaSerial.begin(
    115200,
    SERIAL_8N1,
    LORA_RX_PIN,
    LORA_TX_PIN
  );

  delay(2000);


  // ==========================================
  // STARTUP MESSAGE
  // ==========================================

  Serial.println();
  Serial.println("================================");
  Serial.println("     TANK SYNC - TX TEST");
  Serial.println("================================");
  Serial.println();


  // ==========================================
  // RYLR998 DIAGNOSTIC
  // ==========================================

  sendAT("AT");
  sendAT("AT+ADDRESS?");
  sendAT("AT+NETWORKID?");
  sendAT("AT+BAND?");
  sendAT("AT+PARAMETER?");
}

void loop() {

  // ==================================================
  // 1. Measure ultrasonic distance
  // ==================================================

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    Serial.println("Distance: No echo");
  } 
  else {
    float distance = duration / 58.0;

    Serial.print("Distance: ");
    Serial.print(distance, 1);
    Serial.println(" cm");
  }


  // ==================================================
  // 2. Check for anything received from RYLR998
  // ==================================================

  while (LoRaSerial.available()) {
    Serial.print("LoRa RX: ");
    Serial.write(LoRaSerial.read());
  }

  delay(1000);
}