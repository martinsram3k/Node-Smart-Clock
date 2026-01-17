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

TFT_eSPI tft = TFT_eSPI();

// --- WIFI ÚDAJE ---
const char *ssid = "martinsramekhot";
const char *password = "martin123";

// --- NASTAVENÍ ČASU ---
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;     // GMT+1 (Zima)
const int daylightOffset_sec = 3600; // +1h (Léto)

// OpenWeather nastavení
String apiKey = "05f663e9e12e9e58ff629e70d5a33ed5";
String city = "Prague"; // Zde si napiš své město
String countryCode = "CZ";

// Proměnné pro data
float outdoorTemp = 0;
int outdoorHumidity = 0;
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 600000; // Aktualizace každých 10 minut (šetříme API)

// --- BARVY PRO TVŮJ DISPLEJ (OPRAVENÉ POŘADÍ) ---
#define BGR_BLACK 0x0000     // Černá
#define BGR_WHITE 0xFFFF     // Bílá
#define BGR_BLUE 0xF800      // Modrá (původně kód pro červenou)
#define BGR_RED 0x001F       // Červená (původně kód pro modrou)
#define BGR_GREEN 0x07E0     // Zelená (zůstává stejná)
#define BGR_YELLOW 0x07FF    // Žlutá
#define BGR_CYAN 0xFFE0      // Azurová
#define BGR_MAGENTA 0xF81F   // Fialová/Růžová
#define BGR_ORANGE 0x03FF    // Oranžová
#define BGR_DARKGREY 0x7BEF  // Tmavě šedá
#define BGR_LIGHTGREY 0xC618 // Světle šedá
#define BGR_LIGHTBLUE 0x3800 // Světle modrá (pro tvůj displej)

// --- PINY ---
const int CLK_PIN = 26;
const int DT_PIN = 27;
const int SW_PIN = 14;
const int DISPLAY_BUTTON_PIN = 25;
const int TOUCH_PIN = 15;

const int resolution = 2;
int currentPageIndex = 0;
int lastPageIndex = -1; // Pro detekci změny stránky (proti blikání)
int inout = 0;          // 0 = inside, 1 = outside

Adafruit_AHTX0 aht;
float lastTemp = 0;
float lastHum = 0;
unsigned long lastSensorRead = 0;

ESP32Encoder encoder;
unsigned long lastPress = 0;
const int debounceDelay = 250;

// Funkce pro získání času
String getFormattedTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    return "??:??";
  }
  char timeStringBuff[6];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M", &timeinfo);
  return String(timeStringBuff);
}

void updateWeatherData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    // Sestavení URL adresy pro API volání
    String url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&units=metric&appid=" + apiKey;
    
    http.begin(url);
    int httpCode = http.GET(); // Provede HTTP GET požadavek

    if (httpCode > 0) {
      String payload = http.getString();
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload); // Rozbalení JSON dat

      // Načtení konkrétních hodnot z JSON struktury
      outdoorTemp = doc["main"]["temp"];
      outdoorHumidity = doc["main"]["humidity"];
    }
    http.end();
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  // 1. Inicializace I2C a Displeje
  Wire.begin(21, 22);
  tft.init();
  /*   tft.setSwapBytes(true); // Prohodí pořadí bajtů, aby barvy odpovídaly RGB standardu
   */
  tft.setRotation(1);       // 1 = Na šířku (320x240)
  tft.invertDisplay(false); // ST7789 často vyžaduje invertované barvy
  tft.fillScreen(BGR_BLACK);

  // 2. Připojení k WiFi (Nutné pro čas)
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
  }
  else
  {
    tft.println("WiFi selhala!");
  }

  delay(1000);
  tft.fillScreen(BGR_BLACK);

  if (!aht.begin())
  {
    Serial.println("AHT21 nenalezen!");
  }
  else
  {
    Serial.println("AHT21 OK");
  }

  // 3. Vstupy
  pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(DISPLAY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(TOUCH_PIN, INPUT);
  encoder.attachHalfQuad(DT_PIN, CLK_PIN);
  encoder.setCount(0);

  Serial.println(">>> System pripraven <<<");
}

void loop()
{
  unsigned long now = millis();

  // --- D. LOGIKA TLAČÍTEK ---
  if (digitalRead(SW_PIN) == LOW && (now - lastPress > debounceDelay))
  {
    if (currentPageIndex == 1)
    {
      inout = (inout == 0) ? 1 : 0; // Přepne 0 na 1 nebo 1 na 0

      // Okamžité překreslení nápisu bez čekání na enkodér
      tft.fillRect(0, 50, 320, 40, BGR_BLACK); // Smaže jen oblast nápisu Inside/Outside
      tft.setTextColor(BGR_WHITE, BGR_BLACK);
      tft.setTextDatum(MC_DATUM);
      String label = (inout == 1) ? "Outside" : "Inside";
      tft.drawString(label, tft.width() / 2, 60, 2);
      tft.drawString("Endofer SW pro zmenu", tft.width() / 2, 90, 1);
    }
    lastPress = now;
  }

  if (digitalRead(DISPLAY_BUTTON_PIN) == LOW && (now - lastPress > debounceDelay))
  {
    Serial.println("Klik: Display Button");
    lastPress = now;
  }

  if (digitalRead(TOUCH_PIN) == HIGH && (now - lastPress > debounceDelay))
  {
    Serial.println("Klik: TTP223 Touch");
    lastPress = now;
  }

  // --- A. AKTUALIZACE ČASU (Každou vteřinu) ---
  static unsigned long lastTimeUpdate = 0;
  if (now - lastTimeUpdate >= 1000)
  {
    lastTimeUpdate = now;
    tft.setTextColor(BGR_WHITE, BGR_BLACK); // Bílý čas, černé pozadí vymaže starý
    tft.setTextDatum(TL_DATUM);             // Top Left
    tft.drawString(getFormattedTime(), 10, 10, 2);
    tft.drawLine(0, 45, tft.width(), 45, BGR_WHITE); // Oddělovací čára
  }

  // --- B. LOGIKA ENKODÉRU ---
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

  // --- C. VYKRESLOVÁNÍ STRÁNKY (Jen při změně = nebliká) ---
  // --- C. VYKRESLOVÁNÍ STRÁNKY (Tvůj styl, ale bez blikání) ---
  if (currentPageIndex != lastPageIndex)
  {
    lastPageIndex = currentPageIndex;

    // Vymažeme spodní část obrazovky pod časem (od pixelu 40 dolů)
    // Tím zajistíme, že tam nezůstane text z předchozí stránky
    tft.fillRect(0, 40, 320, 200, BGR_BLACK);

    if (currentPageIndex == 0)
    {
      tft.fillScreen(BGR_BLACK); // Vyčistit celý disple

      tft.setTextColor(BGR_WHITE);
      tft.setTextDatum(TR_DATUM); // Top Left
      tft.drawString("Home", tft.width() - 10, 10, 2);
    }
  else if (currentPageIndex == 1)
    {
      tft.fillScreen(BGR_BLACK);
      tft.setTextColor(BGR_WHITE, BGR_BLACK);
      tft.setTextDatum(TR_DATUM);
      tft.drawString("Weather", tft.width() - 10, 10, 2);
      
      // Vykreslení aktuálního stavu inout
      tft.setTextDatum(MC_DATUM);
      String label = (inout == 1) ? "Outside" : "Inside";
      tft.drawString(label, tft.width() / 2, 60, 2);
      tft.drawString("Endofer SW pro zmenu", tft.width() / 2, 90, 1);
      
      lastPageIndex = currentPageIndex; 
    }
    else if (currentPageIndex == 2)
    {
      tft.fillScreen(BGR_BLACK); // Vyčistit celý disple

      tft.setTextColor(BGR_WHITE);
      tft.setTextDatum(TR_DATUM); // Top Left
      tft.drawString("BTC", tft.width() - 10, 10, 2);
    }
    else if (currentPageIndex == 3)
    {
      tft.fillScreen(BGR_BLACK); // Vyčistit celý disple

      tft.setTextColor(BGR_WHITE);
      tft.setTextDatum(TR_DATUM); // Top Left
      tft.drawString("Settings", tft.width() - 10, 10, 2);
    }
  }

  if (currentPageIndex == 1)
  {

    if (inout == 0) {
static unsigned long lastWeatherUpdate = 0;
    if (millis() - lastWeatherUpdate > 3000)
    {
      lastWeatherUpdate = millis();
      sensors_event_t humidity, temp;
      aht.getEvent(&humidity, &temp);

      tft.setTextDatum(BL_DATUM);
      tft.setTextColor(BGR_WHITE, BGR_BLACK);

      tft.drawString("Teplota:", 10, tft.height() - 40, 2);
      String tStr = String(temp.temperature, 1) + " C";
      if (temp.temperature >= 29.0)
      {
        tft.setTextColor(BGR_RED, BGR_BLACK);
      }
      else if (temp.temperature <= 18.0)
      {
        tft.setTextColor(BGR_BLUE, BGR_BLACK);
      }
      else
      {
        tft.setTextColor(BGR_GREEN, BGR_BLACK);
      }
      tft.drawString(tStr, 10, tft.height() - 10, 2);

      tft.setTextDatum(BR_DATUM);
      tft.setTextColor(BGR_WHITE, BGR_BLACK);
      tft.drawString("Vlhkost:", tft.width() - 10, tft.height() - 40, 2);
      String hStr = String(humidity.relative_humidity, 0) + " %";
      tft.setTextColor(BGR_BLUE, BGR_BLACK);
      tft.drawString(hStr, tft.width() - 10, tft.height() - 10, 2);
    }
    } else {
      // Simulovaná data pro venkovní prostředí
      tft.setTextDatum(BL_DATUM);
      tft.setTextColor(BGR_WHITE, BGR_BLACK);

      tft.drawString("Teplota:", 10, tft.height() - 40, 2);
      String tStr = "22.5 C"; // Simulovaná teplota
      tft.setTextColor(BGR_GREEN, BGR_BLACK);
      tft.drawString(tStr, 10, tft.height() - 10, 2);

      tft.setTextDatum(BR_DATUM);
      tft.setTextColor(BGR_WHITE, BGR_BLACK);
      tft.drawString("Vlhkost:", tft.width() - 10, tft.height() - 40, 2);
      String hStr = "55 %"; // Simulovaná vlhkost
      tft.setTextColor(BGR_BLUE, BGR_BLACK);
      tft.drawString(hStr, tft.width() - 10, tft.height() - 10, 2);
    }
    
  }

  delay(5);
}