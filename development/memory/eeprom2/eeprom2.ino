// TestEEPROM.h
#ifndef TEST_EEPROM_H
#define TEST_EEPROM_H

#include <EEPROM.h>
#include <Arduino.h>

// Exactement comme DFRobot
#define EEPROM_write(address, p) {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, p)  {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) pp[i]=EEPROM.read(address+i);}

class TestEEPROM {
public:
    TestEEPROM() {}
    
    void begin() {
        // Copié directement de DFRobot_PH::begin()
        EEPROM_read(TEST_ADDR, _testValue);
        Serial.print("Loaded value:");
        Serial.println(_testValue);
        
        if(EEPROM.read(TEST_ADDR) == 0xFF) {
            _testValue = 123.45;  // Valeur par défaut
            EEPROM_write(TEST_ADDR, _testValue);
            Serial.println("New EEPROM, write default value");
        }
    }
    
    void save(float newValue) {
        _testValue = newValue;
        EEPROM_write(TEST_ADDR, _testValue);
        Serial.print("Saved value: ");
        Serial.println(_testValue);
    }
    
    float read() {
        EEPROM_read(TEST_ADDR, _testValue);
        return _testValue;
    }

private:
    static const int TEST_ADDR = 0;
    float _testValue;
};

#endif

// TestEEPROM.ino
TestEEPROM tester;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== Test EEPROM ===");
    tester.begin();
    Serial.println("\nCommandes:");
    Serial.println("s: Sauvegarder 987.65");
    Serial.println("r: Lire valeur");
}

void loop() {
    if (Serial.available()) {
        char cmd = Serial.read();
        switch(cmd) {
            case 's':
                Serial.println("\nSauvegarde...");
                tester.save(987.65);
                break;
            
            case 'r':
                Serial.println("\nLecture...");
                float val = tester.read();
                Serial.print("Valeur: ");
                Serial.println(val);
                break;
        }
    }
}