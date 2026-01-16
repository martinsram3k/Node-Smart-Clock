/*
 * Projekt: ESP32 Enkodér (Směr 0/1) + Tlačítka + TTP223
 * Piny: CLK=26, DT=27, SW=14, Button=25, TTP=15
 */

#include <Arduino.h>
#include <ESP32Encoder.h>

// Definice pinů
const int CLK_PIN = 26;
const int DT_PIN = 27;
const int SW_PIN = 14;              // Tlačítko na enkodéru
const int DISPLAY_BUTTON_PIN = 25;  // Externí tlačítko
const int TOUCH_PIN = 15;           // Dotykový senzor TTP223

// Instance a proměnné
ESP32Encoder encoder;
long lastEncoderValue = 0;
unsigned long lastPress = 0;
const int debounceDelay = 250;      // Ošetření zákmitů v ms

void setup() {
  Serial.begin(115200);

  // Nastavení vstupů
  pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(DISPLAY_BUTTON_PIN, INPUT_PULLUP);
  pinMode(TOUCH_PIN, INPUT);        // TTP223 posílá vlastní logické úrovně

  // Inicializace enkodéru (bez vnitřních pull-upů, jak jsi testoval)
  encoder.attachHalfQuad(DT_PIN, CLK_PIN);
  encoder.setCount(0);

  Serial.println(">>> System pripraven <<<");
  Serial.println("Otocenim zjistis smer (0 vlevo / 1 vpravo)");
}

void loop() {
  // --- 1. LOGIKA ENKODÉRU (SMĚR) ---
  long currentCount = encoder.getCount();

  if (currentCount != lastEncoderValue) {
    if (currentCount > lastEncoderValue) {
      Serial.println("Smer: 1"); // Doprava
    } 
    else {
      Serial.println("Smer: 0"); // Doleva
    }

    // Resetování čítače pro další detekci pohybu
    encoder.setCount(0);
    lastEncoderValue = 0;
  }

  // --- 2. LOGIKA TLAČÍTEK (DEBOUNCED) ---
  unsigned long now = millis();

  // Tlačítko enkodéru (aktivní v nule)
  if (digitalRead(SW_PIN) == LOW && (now - lastPress > debounceDelay)) {
    Serial.println("Klik: Encoder SW");
    lastPress = now;
  }

  // Externí tlačítko (aktivní v nule)
  if (digitalRead(DISPLAY_BUTTON_PIN) == LOW && (now - lastPress > debounceDelay)) {
    Serial.println("Klik: Display Button");
    lastPress = now;
  }

  // Dotykový senzor (aktivní v jedničce)
  if (digitalRead(TOUCH_PIN) == HIGH && (now - lastPress > debounceDelay)) {
    Serial.println("Klik: TTP223 Touch");
    lastPress = now;
  }

  delay(5); // Malá pauza pro stabilitu CPU
}