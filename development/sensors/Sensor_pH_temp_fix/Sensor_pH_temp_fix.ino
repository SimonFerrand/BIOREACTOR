#include "DFRobot_PH.h"
#include <EEPROM.h>
#include <SoftwareSerial.h>

#define PH_PIN A1
#define RX_PIN 2  // Broche RX pour la communication avec Teensy
#define TX_PIN 3  // Broche TX pour la communication avec Teensy

float voltage, phValue, temperature;
DFRobot_PH ph;

const float fixedTemperature = 25.0;

SoftwareSerial teensySerial(RX_PIN, TX_PIN); // RX, TX pour la communication avec Teensy

void setup() {
  Serial.begin(9600);  // Communication série avec l'ordinateur pour le débogage
  teensySerial.begin(4800);  // Communication série logicielle avec le Teensy 4.1
  ph.begin();
  
  Serial.println("Arduino prêt à mesurer le pH");
}

void loop() {
  static unsigned long timepoint = millis();

  if(millis() - timepoint > 1000U) {
    timepoint = millis();
    
    temperature = fixedTemperature;
    voltage = analogRead(PH_PIN) / 1024.0 * 5000;  // 5000 pour 5V
    phValue = ph.readPH(voltage, temperature);
    
    // Envoyer les données au Teensy 4.1 via le port série logiciel
    teensySerial.print("pH:");
    teensySerial.println(phValue, 2);
    
    // Afficher sur le port série matériel pour le débogage
    Serial.print("Température (fixe): ");
    Serial.print(temperature, 1);
    Serial.print("°C  pH: ");
    Serial.println(phValue, 2);
  }

  ph.calibration(voltage, temperature);
}