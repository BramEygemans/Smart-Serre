
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <WiFi.h>
#include <WebServer.h>

// =================================
// WIFI INSTELLINGEN
// =================================

const char* ssid = "Automation";
const char* password = "Automation";

WebServer server(80);

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
// WIFI VERBINDEN
// =================================

void verbindMetWifi() {

  Serial.println("Verbinden met wifi...");

  WiFi.begin(ssid, password);

  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wifi verbonden!");
    Serial.print("IP-adres: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Wifi verbinding mislukt!");
  }
}

// =================================
// WEBSITE
// =================================

void toonWebsite() {

  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="nl">

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">

  <title>Smart Greenhouse</title>

  <style>
    body {
      font-family: Arial, sans-serif;
      background: #eaf2e5;
      margin: 0;
      padding: 20px;
      text-align: center;
    }

    h1 {
      color: #245c35;
      margin-bottom: 5px;
    }

    .ondertitel {
      color: #555;
      margin-bottom: 25px;
    }

    .kaart {
      background: white;
      max-width: 400px;
      margin: 15px auto;
      padding: 20px;
      border-radius: 12px;
      box-shadow: 0 2px 8px #0002;
    }

    .waarde {
      font-size: 30px;
      font-weight: bold;
      color: #245c35;
    }

    .eenheid {
      color: #777;
    }

    button {
      padding: 12px 18px;
      margin: 6px;
      border: none;
      border-radius: 8px;
      background: #31834a;
      color: white;
      font-size: 15px;
      cursor: pointer;
    }

    button:hover {
      background: #245c35;
    }

    .status {
      font-weight: bold;
    }
  </style>
</head>

<body>

  <h1>Smart Greenhouse</h1>
  <p class="ondertitel">Monitoring en bediening van de serre</p>

  <div class="kaart">
    <h2>Bodemvocht</h2>
    <div class="waarde" id="bodem">--</div>
  </div>

  <div class="kaart">
    <h2>Lichtsterkte</h2>
    <div class="waarde" id="licht">--</div>
    <p class="eenheid">lux</p>
  </div>

  <div class="kaart">
    <h2>Temperatuur</h2>
    <div class="waarde" id="temperatuur">--</div>
    <p class="eenheid">°C</p>
  </div>

  <div class="kaart">
    <h2>LoRa verbinding</h2>
    <p class="status" id="verbinding">Controleren...</p>
    <p id="berichten">Ontvangen berichten: 0</p>
  </div>

  <div class="kaart">
    <h2>Bediening</h2>

    <h3>Verlichting</h3>
    <button onclick="commando('CMD;LIGHT;1')">Licht AAN</button>
    <button onclick="commando('CMD;LIGHT;0')">Licht UIT</button>

    <h3>Waterindicatie</h3>
    <button onclick="commando('CMD;WATER;1')">Water AAN</button>
    <button onclick="commando('CMD;WATER;0')">Water UIT</button>

    <h3>Dakluik</h3>
    <button onclick="commando('CMD;ROOF;OPEN')">Dakluik openen</button>

    <p id="melding"></p>
  </div>

  <script>

    async function updateData() {

      try {

        const response = await fetch('/data');
        const data = await response.json();

        document.getElementById('bodem').innerText =
          data.bodemvocht;

        document.getElementById('licht').innerText =
          data.licht;

        document.getElementById('temperatuur').innerText =
          data.temperatuur;

        document.getElementById('verbinding').innerText =
          data.verbonden ? 'Verbonden' : 'Geen verbinding';

        document.getElementById('verbinding').style.color =
          data.verbonden ? 'green' : 'red';

        document.getElementById('berichten').innerText =
          'Ontvangen berichten: ' + data.berichten;

      } catch (error) {

        document.getElementById('verbinding').innerText =
          'Server niet bereikbaar';

      }
    }

    async function commando(waarde) {

      try {

        const response = await fetch(
          '/cmd?waarde=' + encodeURIComponent(waarde)
        );

        const tekst = await response.text();

        document.getElementById('melding').innerText = tekst;

      } catch (error) {

        document.getElementById('melding').innerText =
          'Commando kon niet verzonden worden';

      }
    }

    updateData();
    setInterval(updateData, 3000);

  </script>

</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

// =================================
// MEETWAARDEN NAAR WEBSITE
// =================================

void geefData() {

  String json = "{";

  json += "\"bodemvocht\":" + String(bodemvocht) + ",";
  json += "\"licht\":" + String(licht, 1) + ",";
  json += "\"temperatuur\":" + String(temperatuur, 1) + ",";
  json += "\"verbonden\":";
  json += (serreVerbonden ? "true" : "false");
  json += ",";
  json += "\"berichten\":" + String(aantalBerichten);

  json += "}";

  server.send(200, "application/json", json);
}

// =================================
// WEBSITE COMMANDO ONTVANGEN
// =================================

void ontvangWebsiteCommando() {

  if (!server.hasArg("waarde")) {
    server.send(400, "text/plain", "Geen commando ontvangen");
    return;
  }

  String commando = server.arg("waarde");

  if (verstuurCommando(commando)) {
    server.send(200, "text/plain", "Commando verzonden: " + commando);
  } else {
    server.send(400, "text/plain", "Ongeldig commando");
  }
}

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
// TFT BIJWERKEN
// =================================

void toonData() {

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(false);

  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);
  tft.setCursor(5, 5);
  tft.println("SMART SERRE");

  tft.drawFastHLine(5, 28, 150, ST77XX_WHITE);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);

  tft.setCursor(5, 40);
  tft.print("Bodemvocht: ");

  tft.setTextColor(ST77XX_CYAN);
  tft.print(bodemvocht);

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

  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(5, 80);
  tft.print("Temperatuur: ");

  tft.setTextColor(ST77XX_CYAN);
  tft.print(temperatuur, 1);
  tft.print(" C");

  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(5, 105);
  tft.print("LoRa: VERBONDEN");

  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(5, 118);
  tft.print("Berichten: ");
  tft.print(aantalBerichten);
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
// VERBINDINGSCONTROLE
// =================================

void controleerVerbinding() {

  if (serreVerbonden &&
      millis() - laatsteOntvangst > 10000) {

    serreVerbonden = false;

    toonStartscherm();

    tft.setTextColor(ST77XX_RED);
    tft.setCursor(5, 105);
    tft.println("Geen verbinding");

    Serial.println("Geen data meer ontvangen!");
  }
}

// =================================
// COMMANDO VERSTUREN
// =================================

bool verstuurCommando(String commando) {

  commando.trim();

  if (commando != "CMD;LIGHT;1" &&
      commando != "CMD;LIGHT;0" &&
      commando != "CMD;WATER;1" &&
      commando != "CMD;WATER;0" &&
      commando != "CMD;ROOF;OPEN") {

    Serial.println("Ongeldig commando!");
    return false;
  }

  LoRa.println(commando);

  Serial.print("Commando verzonden: ");
  Serial.println(commando);

  return true;
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

  // Wifi verbinden
  verbindMetWifi();

  // Webserver instellen
  server.on("/", toonWebsite);
  server.on("/data", geefData);
  server.on("/cmd", ontvangWebsiteCommando);

  server.begin();

  Serial.println("Webserver gestart!");

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

  // Webserver bedienen
  server.handleClient();

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
