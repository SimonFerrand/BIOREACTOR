#include <EEPROM.h>

// Structure pour les données de test
struct TestData {
    byte signature;      // Signature pour vérifier la validité
    float floatValue;    // Valeur float
    int intValue;       // Valeur int
};

const byte VALID_SIGNATURE = 0xAB;  // Signature arbitraire
const int TEST1_ADDR = 0;          // Adresse pour test 1
const int TEST2_ADDR = sizeof(TestData);  // Adresse pour test 2

// Fonction pour sauvegarder les données
void saveToEEPROM(int address, float floatVal, int intVal) {
    TestData data;
    data.signature = VALID_SIGNATURE;
    data.floatValue = floatVal;
    data.intValue = intVal;
    
    // Écriture octet par octet
    byte* p = (byte*)&data;
    for(unsigned int i = 0; i < sizeof(TestData); i++) {
        EEPROM.write(address + i, p[i]);
    }
    
    Serial.println(F("\nDonnées sauvegardées à l'adresse 0x"));
    Serial.print(address, HEX);
    Serial.println(F(":"));
    Serial.print(F("Float: ")); 
    // Affichage avec 2 décimales, mais sauvegarde la valeur complète
    Serial.println(floatVal, 2);
    Serial.print(F("Int: ")); 
    Serial.println(intVal);
}

// Fonction pour charger les données
bool loadFromEEPROM(int address, float& floatVal, int& intVal) {
    TestData data;
    
    // Lecture octet par octet
    byte* p = (byte*)&data;
    for(unsigned int i = 0; i < sizeof(TestData); i++) {
        p[i] = EEPROM.read(address + i);
    }
    
    // Vérification de la signature
    if(data.signature != VALID_SIGNATURE) {
        Serial.print(F("\nErreur: Données non valides à l'adresse 0x"));
        Serial.println(address, HEX);
        return false;
    }
    
    // Copie des valeurs
    floatVal = data.floatValue;
    intVal = data.intValue;
    
    Serial.print(F("\nDonnées chargées de l'adresse 0x"));
    Serial.print(address, HEX);
    Serial.println(F(":"));
    Serial.print(F("Float: ")); 
    // Affiche d'abord avec 2 décimales puis la valeur complète
    Serial.print(floatVal, 2);
    Serial.print(F(" (valeur complète: "));
    Serial.print(floatVal, 6);
    Serial.println(F(")"));
    Serial.print(F("Int: ")); 
    Serial.println(intVal);
    return true;
}

void printMenu() {
    Serial.println(F("\n=== Menu EEPROM Test ==="));
    Serial.println(F("1: Sauvegarder Test1 (3.14159, 42)"));
    Serial.println(F("2: Sauvegarder Test2 (2.71828, 100)"));
    Serial.println(F("3: Charger Test1"));
    Serial.println(F("4: Charger Test2"));
    Serial.println(F("5: Charger les deux tests"));
    Serial.println(F("h: Afficher ce menu"));
    Serial.println(F("======================"));
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    printMenu();
}

void loop() {
    if(Serial.available()) {
        char cmd = Serial.read();
        float loadedFloat;
        int loadedInt;
        
        switch(cmd) {
            case '1':
                saveToEEPROM(TEST1_ADDR, 3.14159, 42);
                break;
                
            case '2':
                saveToEEPROM(TEST2_ADDR, 2.71828, 100);
                break;
                
            case '3':
                loadFromEEPROM(TEST1_ADDR, loadedFloat, loadedInt);
                break;
                
            case '4':
                loadFromEEPROM(TEST2_ADDR, loadedFloat, loadedInt);
                break;
                
            case '5':
                Serial.println(F("\n=== Chargement des deux tests ==="));
                loadFromEEPROM(TEST1_ADDR, loadedFloat, loadedInt);
                loadFromEEPROM(TEST2_ADDR, loadedFloat, loadedInt);
                break;
                
            case 'h':
                printMenu();
                break;
        }
    }
}