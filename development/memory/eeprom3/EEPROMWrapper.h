#ifndef EEPROM_WRAPPER_H
#define EEPROM_WRAPPER_H

#include <Arduino.h>
#include <EEPROM.h>

class EEPROMWrapper {
public:
    // Structure de test simple
    struct TestData {
        float voltage;
        float temperature;
        byte isInitialized;
    };

    // Méthodes statiques pour sauvegarder/charger la structure
    static void saveData(int address, TestData& data) {
        Serial.println(F("\n=== Saving Data ==="));
        Serial.print(F("Address: ")); Serial.println(address);
        Serial.print(F("Voltage: ")); Serial.println(data.voltage);
        Serial.print(F("Temperature: ")); Serial.println(data.temperature);
        
        EEPROM.put(address, data);
        
        // Vérification immédiate
        TestData verify;
        EEPROM.get(address, verify);
        Serial.println(F("\n=== Verification ==="));
        Serial.print(F("Read back voltage: ")); Serial.println(verify.voltage);
        Serial.print(F("Read back temp: ")); Serial.println(verify.temperature);
    }

    static bool loadData(int address, TestData& data) {
        Serial.println(F("\n=== Loading Data ==="));
        Serial.print(F("From address: ")); Serial.println(address);
        
        EEPROM.get(address, data);
        
        Serial.print(F("Loaded voltage: ")); Serial.println(data.voltage);
        Serial.print(F("Loaded temperature: ")); Serial.println(data.temperature);
        Serial.print(F("Initialized flag: 0x")); Serial.println(data.isInitialized, HEX);
        
        return data.isInitialized == 0xAB;
    }

    static void initializeData(int address, TestData& data) {
        data.voltage = 123.45f;
        data.temperature = 25.0f;
        data.isInitialized = 0xAB;
        saveData(address, data);
    }

    static void clearData(int address) {
        Serial.println(F("\n=== Clearing Data ==="));
        for(int i = 0; i < sizeof(TestData); i++) {
            EEPROM.write(address + i, 0xFF);
            Serial.print(F("Cleared address ")); 
            Serial.print(address + i);
            Serial.print(F(": 0x"));
            Serial.println(EEPROM.read(address + i), HEX);
        }
    }
};

#endif