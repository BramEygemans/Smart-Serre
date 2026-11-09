
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// =================================
// LORA PINNEN
// =================================

#define LORA_RX 16
#define LORA_TX 17

HardwareSerial LoRa(2);

// =================================
// TFT PINNEN
// =================================

#define TFT_SCLK 18
#define TFT_MOSI 19
#define TFT_CS   13
#define TFT_DC   14
#define TFT_RST  23

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// =================================
// VARIABELEN
// =================================

String ontvangenBericht = "";

int bodemvocht = 0;
float licht = 0.0;
float temperatuur = 0.0;

int aantalBerichten = 0;
bool serreVerbonden = false;

unsigned long laatsteOntvangst = 0;

// =================================
// TFT STARTSCHERM
// =================================

void toonStartscherm() {

  tft.fillScreen(ST77XX_BLACK);

  tft.setTextWrap(false);

  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(5, 5);
  tft.println("SMART SERRE");

  tft.drawFastHLine(5, 28, 150, ST77XX_WHITE);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);

  tft.setCursor(5, 45);
  tft.println("Wachten op serre...");

  tft.setCursor(5, 65);
  tft.println("Bodem: --");

  tft.setCursor(5, 80);
  tft.println("Licht: -- lux");

  tft.setCursor(5, 95);
  tft.println("Temp: -- C");
}

// =================================
// DATA UIT BERICHT HALEN
// =================================

String haalWaarde(String bericht, String sleutel) {

  int start = bericht.indexOf(sleutel + "=");

  if (start == -1) {
    return "";
  }

  start += sleutel.length() + 1;

  int einde = bericht.indexOf(';', start);

  if (einde == -1) {
    einde = bericht.length();
  }

  return bericht.substring(start, einde);
}

// =================================
// DATA VERWERKEN
// =================================

void verwerkData(String bericht) {

  if (!bericht.startsWith("DATA;")) {
    Serial.println("Onbekend bericht ontvangen.");
    return;
  }

  String bodem = haalWaarde(bericht, "bodemvocht");
  String lux = haalWaarde(bericht, "licht");
  String temp = haalWaarde(bericht, "temperatuur");

  if (bodem == "" || lux == "" || temp == "") {
    Serial.println("Fout: onvolledig DATA-bericht.");
    return;
  }

  bodemvocht = bodem.toInt();
  licht = lux.toFloat();
  temperatuur = temp.toFloat();

  aantalBerichten++;
  laatsteOntvangst = millis();
  serreVerbonden = true;

  Serial.println("----------------------------");
  Serial.println("DATA ONTVANGEN");
  Serial.print("Bodemvocht: ");
  Serial.println(bodemvocht);

  Serial.print("Licht: ");
  Serial.println(licht);

  Serial.print("Temperatuur: ");
  Serial.println(temperatuur);

  toonData();
}

// =================================
// TFT BIJWERKEN
// =================================

void toonData() {

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(false);

  // Titel
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(5, 5);
  tft.println("SMART SERRE");

  tft.drawFastHLine(5, 28, 150, ST77XX_WHITE);

  // Bodemvocht
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(5, 40);
  tft.print("Bodemvocht: ");

  tft.setTextColor(ST77XX_CYAN);
  tft.print(bodemvocht);

  // Licht
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(5, 60);
  tft.print("Licht: ");

  if (licht >= 0.0) {
    tft.setTextColor(ST77XX_YELLOW);
    tft.print(licht, 1);
    tft.print(" lux");
  } else {
    tft.setTextColor(ST77XX_RED);
    tft.print("Sensorfout");
  }

  // Temperatuur
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(5, 80);
  tft.print("Temperatuur: ");

  tft.setTextColor(ST77XX_CYAN);
  tft.print(temperatuur, 1);
  tft.print(" C");

  // Verbinding
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(5, 105);
  tft.print("LoRa: VERBONDEN");

  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(5, 118);
  tft.print("Berichten: ");
  tft.print(aantalBerichten);
}

// =================================
// TFT VERBINDINGSCONTROLE
// =================================

void controleerVerbinding() {

  if (serreVerbonden &&
      millis() - laatsteOntvangst > 10000) {

    serreVerbonden = false;

    toonStartscherm();

    tft.setTextColor(ST77XX_RED);
    tft.setCursor(5, 105);
    tft.println("LoRa: VERBONDEN?");

    Serial.println("Geen data meer ontvangen!");
  }
}

// =================================
// COMMANDO VERSTUREN
// =================================

void verstuurCommando(String commando) {

  commando.trim();

  if (commando != "CMD;LIGHT;1" &&
      commando != "CMD;LIGHT;0" &&
      commando != "CMD;WATER;1" &&
      commando != "CMD;WATER;0" &&
      commando != "CMD;ROOF;OPEN") {

    Serial.println("Ongeldig commando!");
    return;
  }

  LoRa.println(commando);

  Serial.print("Commando verzonden: ");
  Serial.println(commando);
}

// =================================
// SETUP
// =================================

void setup() {

  Serial.begin(115200);

  Serial.setTimeout(100);

  // LoRa starten
  LoRa.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);

  // TFT starten
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);

  toonStartscherm();

  Serial.println("==========================");
  Serial.println("SERVER ESP32 GESTART");
  Serial.println("==========================");

  Serial.println("Commando's via Serial Monitor:");
  Serial.println("CMD;LIGHT;1");
  Serial.println("CMD;LIGHT;0");
  Serial.println("CMD;WATER;1");
  Serial.println("CMD;WATER;0");
  Serial.println("CMD;ROOF;OPEN");
}

// =================================
// LOOP
// =================================

void loop() {

  // -------------------------------
  // 1. COMMANDO'S VIA SERIAL
  // -------------------------------

  if (Serial.available() > 0) {

    String commando = Serial.readStringUntil('\n');
    commando.trim();

    if (commando.length() > 0) {
      verstuurCommando(commando);
    }
  }

  // -------------------------------
  // 2. DATA VIA LORA ONTVANGEN
  // -------------------------------

  while (LoRa.available() > 0) {

    char c = LoRa.read();

    if (c == '\n') {

      ontvangenBericht.trim();

      if (ontvangenBericht.length() > 0) {
        verwerkData(ontvangenBericht);
      }

      ontvangenBericht = "";
    }

    else if (c != '\r') {

      if (ontvangenBericht.length() < 200) {
        ontvangenBericht += c;
      }
    }
  }

  // -------------------------------
  // 3. VERBINDING CONTROLEREN
  // -------------------------------

  controleerVerbinding();
}
