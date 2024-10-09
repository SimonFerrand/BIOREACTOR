#include <Arduino.h>

void setup() {
  Serial.begin(115200);  // Communication série avec l'ordinateur
  Serial3.begin(9600);   // Communication série avec l'Arduino Uno (broches 14 et 15)
  
  Serial.println("Teensy 4.1 prêt à recevoir les données de pH et d'oxygène sur Serial3");
}

void loop() {
  static String receivedData = "";
  
  while (Serial3.available()) {
    char c = Serial3.read();
    
    if (c == '\n') {
      // Traiter les données reçues
      int pHIndex = receivedData.indexOf("pH:");
      int o2Index = receivedData.indexOf("O2:");
      
      if (pHIndex != -1 && o2Index != -1) {
        String pHString = receivedData.substring(pHIndex + 3, o2Index - 1);
        String o2String = receivedData.substring(o2Index + 3);
        
        float pH = pHString.toFloat();
        float o2 = o2String.toFloat();
        
        // Afficher les valeurs reçues
        Serial.print("pH reçu : ");
        Serial.print(pH, 2);
        Serial.print(", Oxygène dissous reçu : ");
        Serial.println(o2, 2);
      }
      
      receivedData = "";
    } else {
      receivedData += c;
    }
  }
}