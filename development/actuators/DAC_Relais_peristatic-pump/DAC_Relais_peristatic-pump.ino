#include <Wire.h>
#include <Adafruit_MCP4725.h>

// Création d'une instance du DAC MCP4725
Adafruit_MCP4725 dac;

// Débit désiré en ml/min
const float desiredFlowRate = 10.0; // Exemple pour un débit de 105 ml/min avec un tube S19

// Pin pour le relais
const int relayPin = 7; // Assurez-vous que ce pin est disponible et correct

// Initialisation
void setup() {
  Serial.begin(115200);
  dac.begin(0x61); // Initialise le MCP4725 à l'adresse par défaut 0x60
  
  pinMode(relayPin, OUTPUT); // Configure le pin du relais comme sortie
  digitalWrite(relayPin, LOW); // Assurez-vous que le relais est désactivé au démarrage
  // Configuration initiale du DAC pour un débit de 105 ml/min basé sur le tableau des débits
  setPumpFlowRate(desiredFlowRate);
}

// Fonction pour ajuster le débit de la pompe
void setPumpFlowRate(float flowRate) {
  // Convertir le débit ml/min en tension DAC
  uint16_t dacValue = flowRateToDAC(flowRate);
  
  // Définir la tension sur le DAC
  dac.setVoltage(dacValue, false);
  Serial.print("Débit réglé à : ");
  Serial.print(flowRate);
  Serial.println(" ml/min");

  // Contrôler le relais en fonction du débit
  if (flowRate > 0) {
    digitalWrite(relayPin, HIGH); // Activer la pompe si le débit est positif
  } else {
    digitalWrite(relayPin, LOW); // Désactiver la pompe si le débit est zéro
  }
}

// Fonction pour convertir le débit en ml/min en une valeur de tension pour le DAC
uint16_t flowRateToDAC(float flowRate) {
  // Plage de débit possible pour S19: 1 à 105 ml/min à 400 RPM avec un calcul de proportionnalité
  // Exemple: 105 ml/min correspond à la tension maximale de sortie du DAC (supposons 5V)
  // Calculer la valeur DAC proportionnelle (0-4095 pour MCP4725)
  float maxFlowRate = 105.0; // Débit maximal pour le tube S19 à 400 RPM
  float proportion = flowRate / maxFlowRate;
  uint16_t dacValue = proportion * 4095; // Convertir proportion en valeur DAC
  return dacValue;
}

// Boucle principale (vide dans cet exemple)
void loop() {
  // Ici, vous pouvez ajouter un code pour ajuster le débit en fonction de capteurs ou de conditions externes
  // delay(1000); // Pour le test, réajuster le débit chaque seconde (commenté pour l'exemple)
}
