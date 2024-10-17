#include <Wire.h>
#include <Adafruit_MCP4728.h>

Adafruit_MCP4728 mcp;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Wire.begin();

  Serial.println("Test MCP4728 - Sortie maximale sur le canal A");

  if (!mcp.begin(0x60)) {
    Serial.println("Erreur d'initialisation du MCP4728. Vérifiez le câblage!");
    while (1);
  }

  // Configurer le canal A pour la sortie maximale
  mcp.setChannelValue(MCP4728_CHANNEL_A, 4095);  // Valeur maximale pour un DAC 12 bits

  Serial.println("Canal A configuré pour la sortie maximale");
  Serial.println("Mesurez la tension sur la broche de sortie du canal A");
}

void loop() {
  // Rien à faire dans la boucle pour ce test
}