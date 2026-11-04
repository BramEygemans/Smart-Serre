# Smart Serre – IoT Project

## 1. Projectbeschrijving

Smart Greenhouse is een IoT-project waarbij de omgevingsomstandigheden van een serre of terrarium automatisch worden gemeten en opgevolgd.

Het systeem bestaat uit twee ESP32-microcontrollers die draadloos met elkaar communiceren via LoRa (868 MHz).

De eerste ESP32 bevindt zich in de serre en verzamelt gegevens van verschillende sensoren. Deze gegevens worden doorgestuurd naar een tweede ESP32, die als server fungeert. De server ontvangt de meetgegevens, toont ze op een TFT-scherm en kan commando's terugsturen naar de serre.

Het uiteindelijke doel is om de serre gedeeltelijk automatisch te kunnen besturen en de meetgegevens via een website beschikbaar te maken.

## 2. Systeemarchitectuur

Het project bestaat uit twee afzonderlijke modules:

### Serre ESP32 (Sensor Node)

De serre-module verzamelt gegevens en stuurt deze via LoRa naar de server.

**Sensoren:**
- Capacitatieve bodemvochtsensor: meet het vochtgehalte van de bodem.
- BH1750: meet de lichtintensiteit in lux.
- DS18B20: meet de omgevingstemperatuur.

**Actuatoren:**
- Gele LED: simulatie van een extra verlichting.
- Blauwe LED: simulatie van een watergeefsysteem.
- Servo: toekomstige bediening van een dakluik.

### Server ESP32 (Server Node)

De server-module ontvangt de gegevens van de serre via LoRa.

**Functionaliteiten:**
- Ontvangen en verwerken van sensorwaarden.
- Weergeven van meetgegevens op een TFT-scherm.
- Versturen van commando's naar de serre.
- Mogelijkheid om commando's handmatig via de Serial Monitor te versturen.
- Toekomstige uitbreiding met een webserver en online dashboard.

## 3. Hardware

| Component | Aantal | Functie |
|---|---:|---|
| ESP32 | 2 | Microcontrollers |
| EBYTE E220-900T22D | 2 | LoRa-communicatie |
| Capacitatieve bodemvochtsensor | 1 | Bodemvocht meten |
| BH1750 | 1 | Lichtintensiteit meten |
| DS18B20 | 1 | Temperatuur meten |
| TFT ST7735 1.8" | 1 | Weergave meetgegevens |
| LED geel | 1 | Verlichting simuleren |
| LED blauw | 1 | Watergeefsysteem simuleren |
| LED extra | 1 | Statusindicatie BH1750 |
| Servo | 1 | Dakluik bedienen |
| Weerstanden | Verschillende | Pull-up en LED-stroombegrenzing |

## 4. Aansluitingen

### 4.1 Serre ESP32

| Component | ESP32-pin |
|---|---|
| LoRa RXD | GPIO 16 |
| LoRa TXD | GPIO 17 |
| Bodemvochtsensor AOUT | GPIO 34 |
| DS18B20 DATA | GPIO 32 |
| BH1750 SDA | GPIO 21 |
| BH1750 SCL | GPIO 22 |
| Servo | GPIO 26 |
| Gele LED | GPIO 25 |
| Blauwe LED | GPIO 27 |
| BH1750 status-LED | GPIO 33 |

De DS18B20 gebruikt een pull-upweerstand tussen DATA en 3.3 V.

De BH1750 maakt gebruik van de I2C-bus.

### 4.2 Server ESP32

| Component | ESP32-pin |
|---|---|
| LoRa RXD | GPIO 16 |
| LoRa TXD | GPIO 17 |
| TFT SCL (SPI SCK) | GPIO 18 |
| TFT SDA (SPI MOSI) | GPIO 19 |
| TFT RES | GPIO 23 |
| TFT CS | GPIO 13 |
| TFT DC | GPIO 14 |
| TFT BLK | 3.3 V |
| TFT VCC | 3.3 V |
| TFT GND | GND |

**Opmerking:** SCL en SDA op het TFT-scherm worden hier gebruikt als SPI-klok en SPI-data. Dit zijn geen I2C-sign
