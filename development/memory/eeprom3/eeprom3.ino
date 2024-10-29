// test_eeprom_minimal.ino
#include <EEPROM.h>

struct TestData {
    float value1;
    float value2;
    byte initialized;
};

const int EEPROM_ADDR = 0;
TestData data;

void setup() {
    Serial.begin(115200);
    while(!Serial) { }
    delay(1000);
    
    Serial.println(F("=== EEPROM Test Start ==="));
    printDivider();

    loadData();
    printCurrentValues();
    printMenu();
}

void loop() {
    if (Serial.available()) {
        char cmd = Serial.read();
        handleCommand(cmd);
    }
}

void loadData() {
    EEPROM.get(EEPROM_ADDR, data);
    
    if (data.initialized != 0xAB) {
        Serial.println(F("First use - Initializing..."));
        data.value1 = 123.45;
        data.value2 = 67.89;
        data.initialized = 0xAB;
        EEPROM.put(EEPROM_ADDR, data);
    }
}

void handleCommand(char cmd) {
    printDivider();
    
    switch(cmd) {
        case '1':
            Serial.println(F("Saving new values..."));
            data.value1 = 98.76;
            data.value2 = 54.32;
            EEPROM.put(EEPROM_ADDR, data);
            break;
            
        case '2':
            Serial.println(F("Reading values..."));
            loadData();
            break;
            
        case '3':
            Serial.println(F("Clearing EEPROM..."));
            // Effacer
            for(int i = 0; i < sizeof(TestData); i++) {
                EEPROM.write(EEPROM_ADDR + i, 0xFF);
            }
            
            // Relire pour vérifier
            EEPROM.get(EEPROM_ADDR, data);
            Serial.println(F("EEPROM cleared. Raw values:"));
            Serial.print(F("Value 1: ")); Serial.println(data.value1);
            Serial.print(F("Value 2: ")); Serial.println(data.value2);
            Serial.print(F("Initialized: 0x")); Serial.println(data.initialized, HEX);
            
            // Réinitialiser avec les valeurs par défaut
            loadData();
            break;
            
        case 'h':
            printMenu();
            return;
            
        case 'd':  // Nouvelle commande pour debug
            Serial.println(F("Raw EEPROM bytes:"));
            for(int i = 0; i < sizeof(TestData); i++) {
                Serial.print(F("Addr ")); 
                Serial.print(EEPROM_ADDR + i);
                Serial.print(F(": 0x"));
                Serial.println(EEPROM.read(EEPROM_ADDR + i), HEX);
            }
            break;
    }
    
    printCurrentValues();
}

void printCurrentValues() {
    Serial.println(F("\nCurrent values:"));
    Serial.print(F("Value 1: ")); Serial.println(data.value1);
    Serial.print(F("Value 2: ")); Serial.println(data.value2);
    Serial.print(F("Initialized: 0x")); Serial.println(data.initialized, HEX);
}

void printMenu() {
    Serial.println(F("\nCommandes:"));
    Serial.println(F("1: Save new values"));
    Serial.println(F("2: Read values"));
    Serial.println(F("3: Clear EEPROM"));
    Serial.println(F("d: Debug raw bytes"));
    Serial.println(F("h: Show this menu"));
}

void printDivider() {
    Serial.println(F("\n------------------------"));
}