#include <Arduino.h>
#include <ESP32Encoder.h>
#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "time.h"

TFT_eSPI tft = TFT_eSPI();

// --- WIFI ÚDAJE ---
const char *ssid = "martinsramekhot";
const char *password = "martin123";

// --- NASTAVENÍ ČASU ---
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;     // GMT+1 (Zima)
const int daylightOffset_sec = 3600; // +1h (Léto)

// --- BARVY BGR ---
#define BGR_ZLUTA 0x07FF
#define BGR_WHITE 0xFFFF
#define BGR_BLACK 0x0000

// --- PINY ---
const int CLK_PIN = 26;
const int DT_PIN = 27;
const int SW_PIN = 14;
const int DISPLAY_BUTTON_PIN = 25;
const int TOUCH_PIN = 15;

const int resolution = 2;
int currentPageIndex = 0;
int lastPageIndex = -1; // Pro detekci změny stránky (proti blikání)

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

void setup()
{
  Serial.begin(115200);
  delay(1000);

  // 1. Inicializace I2C a Displeje
  Wire.begin(21, 22);
  tft.init();
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

  // --- A. AKTUALIZACE ČASU (Každou vteřinu) ---
  static unsigned long lastTimeUpdate = 0;
  if (now - lastTimeUpdate >= 1000)
  {
    lastTimeUpdate = now;
    tft.setTextColor(BGR_WHITE, BGR_BLACK); // Bílý čas, černé pozadí vymaže starý
    tft.setTextDatum(TL_DATUM);             // Top Left
    tft.drawString(getFormattedTime(), 10, 10, 2);
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
      tft.drawString("Home Page", tft.width() - 10, 10, 2);
    }
    else if (currentPageIndex == 1)
    {
      tft.fillScreen(BGR_BLACK); // Vyčistit celý disple
      tft.setTextColor(BGR_WHITE);
      tft.setTextDatum(TR_DATUM); // Top Left
      tft.drawString("Wheather Page", tft.width() - 10, 10, 2);
    }
    else if (currentPageIndex == 2)
    {
      tft.fillScreen(BGR_BLACK); // Vyčistit celý disple

      tft.setTextColor(BGR_WHITE);
      tft.setTextDatum(TR_DATUM); // Top Left
      tft.drawString("BTC Page", tft.width() - 10, 10, 2);
    }
    else if (currentPageIndex == 3)
    {
      tft.fillScreen(BGR_BLACK); // Vyčistit celý disple

      tft.setTextColor(BGR_WHITE);
      tft.setTextDatum(TR_DATUM); // Top Left
      tft.drawString("Settings Page", tft.width() - 10, 10, 2);
    }
  }

  // --- D. LOGIKA TLAČÍTEK ---
  if (digitalRead(SW_PIN) == LOW && (now - lastPress > debounceDelay))
  {
    Serial.println("Klik: Encoder SW");
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

  delay(5);
}