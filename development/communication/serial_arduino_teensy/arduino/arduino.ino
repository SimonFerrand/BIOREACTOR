#include "DFRobot_PH.h"
#include <EEPROM.h>
#include <SoftwareSerial.h>

#define PH_PIN A1
#define O2_PIN A5  // Supposons que le capteur d'oxygène est connecté à A2
#define RX_PIN 2
#define TX_PIN 3

float voltage, phValue, temperature, o2Value;
DFRobot_PH ph;

const float fixedTemperature = 25.0;

SoftwareSerial teensySerial(RX_PIN, TX_PIN);

// Déclaration des constantes et variables pour le capteur d'oxygène
const uint16_t VREF = 5000;
const uint16_t ADC_RES = 1024;
const uint16_t DO_Table[41] = {
    14460, 14220, 13820, 13440, 13090, 12740, 12420, 12110, 11810, 11530,
    11260, 11010, 10770, 10530, 10300, 10080, 9860, 9660, 9460, 9270,
    9080, 8900, 8730, 8570, 8410, 8250, 8110, 7960, 7820, 7690,
    7560, 7430, 7300, 7180, 7070, 6950, 6840, 6730, 6630, 6530, 6410
};

void setup() {
  Serial.begin(9600);  // Communication série avec l'ordinateur pour le débogage
  teensySerial.begin(9600);  // Communication série avec le Teensy 4.1
  ph.begin();
  
  Serial.println("Arduino prêt à mesurer le pH et l'oxygène dissous");
}

void loop() {
  static unsigned long timepoint = millis();
  
  if(millis() - timepoint > 1000U) {
    timepoint = millis();
    
    temperature = fixedTemperature;
    
    // Mesure du pH
    voltage = analogRead(PH_PIN) / 1024.0 * 5000;  // 5000 pour 5V
    phValue = ph.readPH(voltage, temperature);
    
    // Mesure de l'oxygène dissous
    uint16_t rawO2Value = analogRead(O2_PIN);
    uint32_t o2Voltage = uint32_t(VREF) * rawO2Value / ADC_RES;
    float V_saturation = 1127.6 * pow(0.978, temperature);  // Utilisation de la formule par défaut
    o2Value = (o2Voltage * DO_Table[(int)temperature] / V_saturation);
    
    // Envoyer les données au Teensy 4.1 via le port série logiciel
    teensySerial.print("pH:");
    teensySerial.print(phValue, 2);
    teensySerial.print(",O2:");
    teensySerial.println(o2Value, 2);
    
    // Afficher sur le port série matériel pour le débogage
    Serial.print("Température (fixe): ");
    Serial.print(temperature, 1);
    Serial.print("°C  pH: ");
    Serial.print(phValue, 2);
    Serial.print("  O2: ");
    Serial.println(o2Value, 2);
  }
  
  ph.calibration(voltage, temperature);
}