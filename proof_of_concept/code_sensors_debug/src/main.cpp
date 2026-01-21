#include <Arduino.h>
#include <Wire.h>
#include "ScioSense_ENS160.h" // Standardní knihovna
#include "Custom_ENS160.h"

// --- KONFIGURACE PINŮ ---
// Pokud nefunguje 21/22, zkus 8/9 (jako v tom kódu z internetu)
#define SDA_PIN 21 
#define SCL_PIN 22
#define I2C_FREQUENCY 100000 

// Adresa 0x52 (pokud máš AD0 na GND) nebo 0x53 (pokud máš AD0 na 3.3V)
ScioSense_ENS160 ens160(0x52); 

void setup() {
    Serial.begin(115200);
    while (!Serial);
    
    Serial.println("[ENS160] Startuji inicializaci...");

    // Inicializace I2C s piny nadefinovanými výše
    Wire.begin(SDA_PIN, SCL_PIN, I2C_FREQUENCY);

    // Start senzoru
    if (!ens160.begin()) {
        Serial.println("[CHYBA] ENS160 nenalezen na I2C sběrnici!");
        // Pokud senzor nenajde, vypíše to i v případě špatné adresy (zkus změnit na 0x53)
        while (1) delay(1000); 
    }

    // Nastavení režimu (Standardní provoz)
    ens160.setMode(ENS160_OPMODE_STD);
    Serial.println("[OK] ENS160 inicializován.");
}

void loop() {
    // Příkaz k měření
    ens160.measure();

    // Vyčtení hodnot (základní části z tvého kódu)
    uint16_t eco2 = ens160.geteCO2();
    uint16_t tvoc = ens160.getTVOC();
    uint8_t aqi = ens160.getAQI();

    // Výpis do sériového monitoru
    Serial.print("[loop] ENS160: eCO2: ");
    Serial.print(eco2);
    Serial.print(" ppm, TVOC: ");
    Serial.print(tvoc);
    Serial.print(" ppb, AQI: ");
    Serial.println(aqi);

    // Senzor ENS160 potřebuje čas na výpočet, stačí číst jednou za 2-15 sekund
    delay(5000); 
}