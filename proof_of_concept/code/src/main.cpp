#include <Arduino.h>
#include <ESP32Encoder.h>
#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "time.h"
#include <Adafruit_AHTX0.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "ScioSense_ENS160.h" // Knihovna pro kvalitu vzduchu (Cena cca 150 Kc)
TFT_eSPI tft = TFT_eSPI();
ScioSense_ENS160 ens160(0x52); // Inicializace senzoru na I2C
Adafruit_AHTX0 aht;

// --- WIFI ÚDAJE ---
const char *ssid = "martinsramekhot";
const char *password = "martin123";

// --- NASTAVENÍ ČASU ---
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

// --- OPENWEATHER NASTAVENÍ ---
String apiKey = "05f663e9e12e9e58ff629e70d5a33ed5";
String city = "Pribyslav";
String countryCode = "CZ";

String lokaceMereni = "vnitrni sensory"; // "inside" nebo "outside"

// --- PROMĚNNÉ PRO DATA ---
float outdoorTemp = 0;
int outdoorHumidity = 0;
uint16_t curTVOC = 0;
uint16_t curECO2 = 400;
unsigned long lastEnvRead = 0;
unsigned long lastApiUpdate = 0;
const unsigned long apiInterval = 600000; // 10 minut (600 000 ms)

// --- BARVY PRO TVŮJ DISPLEJ (BGR POŘADÍ) ---
#define BGR_BLACK 0x0000
#define BGR_WHITE 0xFFFF
#define BGR_BLUE 0xF800
#define BGR_RED 0x001F
#define BGR_GREEN 0x07E0
#define BGR_YELLOW 0x07FF
#define BGR_LIGHTBLUE 0x3800

// --- PINY ---
const int CLK_PIN = 26;
const int DT_PIN = 27;
const int SW_PIN = 14;
const int DISPLAY_BUTTON_PIN = 25;
const int TOUCH_PIN = 15;

const int resolution = 2;
int currentPageIndex = 0;
int lastPageIndex = -1;
int inout = 0; // 0 = Inside, 1 = Outside

ESP32Encoder encoder;
unsigned long lastPress = 0;
const int debounceDelay = 250;

// --- FUNKCE PRO ZÍSKÁNÍ ČASU ---
String getFormattedTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
    return "??:??";
  char timeStringBuff[6];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M", &timeinfo);
  return String(timeStringBuff);
}

// --- FUNKCE PRO ZÍSKÁNÍ POČASÍ Z API ---
void updateWeatherData()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    String url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&units=metric&appid=" + apiKey;
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
      String payload = http.getString();
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);
      outdoorTemp = doc["main"]["temp"];
      outdoorHumidity = doc["main"]["humidity"];
      Serial.println("Počasí aktualizováno z API");
    }
    http.end();
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Wire.begin(21, 22);
  tft.init();
  tft.setRotation(1);
  tft.invertDisplay(false);
  tft.fillScreen(BGR_BLACK);

  // Připojení WiFi
  tft.setTextSize(2);
  tft.setTextColor(BGR_WHITE);
  tft.setCursor(10, 10);
  tft.println("Pripojuji WiFi...");

  WiFi.begin(ssid, password);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20)
  {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    tft.println("WiFi OK!");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    updateWeatherData(); // První načtení dat
  }
  else
  {
    tft.println("WiFi selhala!");
  }

  if (!aht.begin())
    Serial.println("AHT21 nenalezen!");

  ens160.begin();
  ens160.setMode(ENS160_OPMODE_STD);
  delay(500);

  // Vynucené nastavení módu (zkusíme to dvakrát)
  ens160.setMode(ENS160_OPMODE_IDLE); // Nejdřív do spánku/nečinnosti
  delay(200);
  ens160.setMode(ENS160_OPMODE_STD); // Pak do standardního měření
  delay(500);

  if (ens160.available())
  {
    Serial.println("ENS160 OK a probuzen.");
  }
  else
  {
    Serial.println("ENS160 nenalezen!");
  }

  pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(DISPLAY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(TOUCH_PIN, INPUT);
  encoder.attachHalfQuad(DT_PIN, CLK_PIN);
  encoder.setCount(0);

  delay(1000);
  tft.fillScreen(BGR_BLACK);
}

void loop()
{
  unsigned long now = millis();

  // --- LOGIKA TLAČÍTEK ---
  if (digitalRead(SW_PIN) == LOW && (now - lastPress > debounceDelay))
  {
    if (currentPageIndex == 1)
    {

      inout = (inout == 0) ? 1 : 0;

      if (inout == 0)
        lokaceMereni = "vnitrni sensory";
      else
        lokaceMereni = city + " sensor";

      // Okamžitý vizuální reset pro Weather stránku
      tft.fillRect(0, 50, 320, 190, BGR_BLACK);
      lastPageIndex = -1; // Vynutí překreslení v sekci C
    }
    lastPress = now;
  }

  // --- AKTUALIZACE ČASU (Každou vteřinu) ---
  static unsigned long lastTimeUpdate = 0;
  if (now - lastTimeUpdate >= 1000)
  {
    lastTimeUpdate = now;
    tft.setTextColor(BGR_WHITE, BGR_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(getFormattedTime(), 10, 10, 2);
    tft.drawLine(0, 45, tft.width(), 45, BGR_WHITE);
  }

  // --- LOGIKA ENKODÉRU ---
  long currentCount = encoder.getCount();
  if (abs(currentCount) >= resolution)
  {
    if (currentCount > 0)
      currentPageIndex++;
    else
      currentPageIndex--;
    if (currentPageIndex > 3)
      currentPageIndex = 0;
    if (currentPageIndex < 0)
      currentPageIndex = 3;
    encoder.setCount(0);
  }

  // --- VYKRESLOVÁNÍ STRÁNKY (Při změně) ---
  if (currentPageIndex != lastPageIndex)
  {
    lastPageIndex = currentPageIndex;
    tft.fillRect(0, 46, 320, 194, BGR_BLACK); // Vyčistit jen spodek

    tft.setTextColor(BGR_WHITE, BGR_BLACK);
    tft.setTextDatum(TR_DATUM);

    if (currentPageIndex == 0)
    {
      tft.fillScreen(BGR_BLACK);
      tft.drawString("Home", 310, 10, 2);
    }
    else if (currentPageIndex == 1)
    {
      tft.fillScreen(BGR_BLACK);
      tft.drawString("Weather", 310, 10, 2);
      tft.setTextDatum(MC_DATUM);
      String label = (inout == 1) ? "Outside" : "Inside";
      tft.drawString(label, 160, 60, 2);
      tft.setTextColor(0x7BEF, BGR_BLACK); // Šedá nápověda
      tft.drawString("Encoder SW pro zmenu", 160, 90, 1);
      tft.setTextColor(0x7BEF, BGR_BLACK);
      tft.drawString(lokaceMereni, 160, 110, 1);
    }
    else if (currentPageIndex == 2)
    {
      tft.fillScreen(BGR_BLACK);
      tft.drawString("Aplications", 310, 10, 2);
    }
    else if (currentPageIndex == 3)
    {
      tft.fillScreen(BGR_BLACK);
      tft.drawString("Settings", 310, 10, 2);
    }
  }

  // --- DYNAMICKÁ AKTUALIZACE DAT ---
  if (currentPageIndex == 1)
  {
    static unsigned long lastDataRefresh = 0;
    // Refresh dat každé 3 vteřiny pro displej
    if (now - lastDataRefresh > 3000)
    {
      lastDataRefresh = now;

      float dTemp, dHum;

      if (inout == 0)
      { // INSIDE (AHT21)
        sensors_event_t humidity, temp;
        aht.getEvent(&humidity, &temp);
        dTemp = temp.temperature;
        dHum = humidity.relative_humidity;

       if (ens160.available()) {
          // measure(false) nezpůsobí zásek, protože nečeká na dokončení
          ens160.measure(false); 

          uint16_t newTVOC = ens160.getTVOC();
          uint16_t newCO2  = ens160.geteCO2();

          // Filtrujeme nesmysly (0xFFFF) a úplné nuly
          if (newCO2 != 0xFFFF && newCO2 > 0) {
            curECO2 = newCO2;
            curTVOC = newTVOC;
          } else if (curECO2 == 0) {
            curECO2 = 400; // Výchozí hodnota pro displej
          }
        }
        
        Serial.print("ENS160: eCO2="); Serial.println(curECO2);

        Serial.print(curECO2);
        Serial.print(" ppm eCO2, ");
        Serial.print(curTVOC);
      }
      else
      { // OUTSIDE (API)
        if (now - lastApiUpdate > apiInterval || lastApiUpdate == 0)
        {
          updateWeatherData();
          lastApiUpdate = now;
        }
        dTemp = outdoorTemp;
        dHum = (float)outdoorHumidity;
      }

      // Vykreslení Teploty (Vlevo dole)
      tft.setTextDatum(BL_DATUM);
      tft.setTextColor(BGR_WHITE, BGR_BLACK);
      tft.drawString("Teplota:", 10, 200, 2);

      if (dTemp >= 29.0)
        tft.setTextColor(BGR_RED, BGR_BLACK);
      else if (dTemp <= 18.0)
        tft.setTextColor(BGR_BLUE, BGR_BLACK);
      else
        tft.setTextColor(BGR_GREEN, BGR_BLACK);

      tft.drawString(String(dTemp, 1) + " C", 10, 230, 2);

      // Vykreslení Vlhkosti (Vpravo dole)
      tft.setTextDatum(BR_DATUM);
      tft.setTextColor(BGR_WHITE, BGR_BLACK);
      tft.drawString("Vlhkost:", 310, 200, 2);
      tft.setTextColor(BGR_BLUE, BGR_BLACK);
      tft.drawString(String(dHum, 0) + " %", 310, 230, 2);
    }
  }

  delay(5);
}