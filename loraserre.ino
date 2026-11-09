
#include <Wire.h>
#include <BH1750.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ================================
// PINNEN
// ================================

// LoRa
#define LORA_RX 16
#define LORA_TX 17

// Sensoren
#define BODEM_PIN 34
#define TEMP_PIN 32

// Toekomstige actuatoren
#define SERVO_PIN 26
#define LED_GEEL 25
#define LED_BLAUW 27
#define LED_BH1750 33

// ================================
// OBJECTEN
// ================================

HardwareSerial LoRa(2);

BH1750 lichtSensor;

OneWire oneWire(TEMP_PIN);
DallasTemperature temperatuurSensor(&oneWire);

// ================================
// VARIABELEN
// ================================

int bodemvocht = 0;
float licht = 0;
float temperatuur = 0;

unsigned long vorigeVerzending = 0;
const unsigned long verzendInterval = 3000;

String ontvangenBericht = "";

// ================================
// SETUP
// ================================

void setup() {

  Serial.begin(115200);

  // LoRa
  LoRa.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);

  // Actuatoren
  pinMode(LED_GEEL, OUTPUT);
  pinMode(LED_BLAUW, OUTPUT);
  pinMode(LED_BH1750, OUTPUT);

  digitalWrite(LED_BH1750, LOW);
  digitalWrite(LED_GEEL, LOW);
  digitalWrite(LED_BLAUW, LOW);

  // I2C
  Wire.begin(21, 22);

  // BH1750
  if (lichtSensor.begin()) {
    Serial.println("BH1750 gevonden");
  } else {
    Serial.println("FOUT: BH1750 niet gevonden");
  }

  // DS18B20
  temperatuurSensor.begin();

  Serial.print("Aantal temperatuursensoren: ");
  Serial.println(temperatuurSensor.getDeviceCount());

  Serial.println("==========================");
  Serial.println("SERRE ESP32 GESTART");
  Serial.println("==========================");
}

// ================================
// SENSORWAARDEN METEN
// ================================

void meetSensoren() {

  bodemvocht = analogRead(BODEM_PIN);

  licht = lichtSensor.readLightLevel();
    if (licht >= 0.0) {
    digitalWrite(LED_BH1750, HIGH);
  } 
  else {
    digitalWrite(LED_BH1750, LOW);
  }
  
  temperatuurSensor.requestTemperatures();

  temperatuur = temperatuurSensor.getTempCByIndex(0);

}

// ================================
// DATA VERZENDEN
// ================================

void verstuurData() {

  meetSensoren();

  String bericht = "DATA;";
  bericht += "bodemvocht=" + String(bodemvocht) + ";";
  bericht += "licht=" + String(licht, 1) + ";";
  bericht += "temperatuur=" + String(temperatuur, 1);

  LoRa.println(bericht);

  Serial.println("Verzonden:");
  Serial.println(bericht);
}

// ================================
// OPDRACHTEN ONTVANGEN
// ================================

void verwerkOpdracht(String bericht) {

  bericht.trim();

  Serial.print("Opdracht ontvangen: ");
  Serial.println(bericht);

  if (bericht == "CMD;LIGHT;1") {
    digitalWrite(LED_GEEL, HIGH);
  }

  else if (bericht == "CMD;LIGHT;0") {
    digitalWrite(LED_GEEL, LOW);
  }

  else if (bericht == "CMD;WATER;1") {
    digitalWrite(LED_BLAUW, HIGH);
  }

  else if (bericht == "CMD;WATER;0") {
    digitalWrite(LED_BLAUW, LOW);
  }

  else if (bericht == "CMD;ROOF;OPEN") {
    Serial.println("Dakluik openen gevraagd.");
    // Servo wordt later toegevoegd.
  }
}

// ================================
// LOOP
// ================================

void loop() {

  // LoRa-berichten ontvangen
  while (LoRa.available()) {

    char c = LoRa.read();

    if (c == '\n') {
      verwerkOpdracht(ontvangenBericht);
      ontvangenBericht = "";
    }

    else if (c != '\r') {
      ontvangenBericht += c;
    }
  }

  // Om de 3 seconden data verzenden
  if (millis() - vorigeVerzending >= verzendInterval) {

    vorigeVerzending = millis();

    verstuurData();
  }
}
