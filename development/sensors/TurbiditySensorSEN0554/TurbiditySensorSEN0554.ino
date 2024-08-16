#include <SoftwareSerial.h>

#define RX_PIN 8
#define TX_PIN 9

SoftwareSerial sensorSerial(RX_PIN, TX_PIN);

unsigned char command[] = {0x18, 0x05, 0x00, 0x01, 0x0D};

// Paramètres d'ajustement
const float SCALE_FACTOR = 1.5;  // Facteur d'échelle
const int OFFSET = 10;           // Décalage

void setup() {
  Serial.begin(115200);
  sensorSerial.begin(9600);
  Serial.println("Capteur de turbidité SEN0554 - Test avec ajustement");
}

void loop() {
  sensorSerial.write(command, 5);
  delay(100);
  
  if (sensorSerial.available() >= 5) {
    unsigned char response[5];
    for (int i = 0; i < 5; i++) {
      response[i] = sensorSerial.read();
    }
    
    if (response[0] == 0x18 && response[1] == 0x05) {
      int rawTurbidity = response[3];
      
      // Appliquer l'ajustement
      float adjustedTurbidity = (rawTurbidity * SCALE_FACTOR) + OFFSET;
      
      Serial.print("Valeur brute de turbidité: ");
      Serial.print(rawTurbidity);
      Serial.print(", Valeur ajustée: ");
      Serial.println(adjustedTurbidity);
    } else {
      Serial.println("Réponse invalide du capteur");
    }
  } else {
    Serial.println("Pas de réponse du capteur");
  }
  
  delay(1000);
}


/*
Je comprends votre question. Vous souhaitez ajuster la sensibilité du capteur ou modifier l'échelle de lecture pour obtenir des valeurs positives même pour de très faibles niveaux de turbidité. Voici quelques informations et suggestions :

Unité de mesure :
La valeur brute renvoyée par le capteur SEN0554 est un nombre entre 0 et 255. Ce n'est pas une unité standard de turbidité comme le NTU (Nephelometric Turbidity Unit). C'est plutôt une échelle arbitraire où 0 représente l'eau la plus claire et 255 l'eau la plus trouble que le capteur peut détecter.
Ajustement de la sensibilité :
Malheureusement, ce capteur ne permet pas d'ajuster sa sensibilité via le code. La sensibilité est déterminée par le hardware du capteur.
Modification de l'échelle de lecture :
Vous pouvez cependant modifier l'échelle de lecture dans votre code pour obtenir des valeurs positives même pour de très faibles niveaux de turbidité.

=>Dans ce code modifié :

SCALE_FACTOR multiplie la valeur brute pour augmenter la sensibilité.
OFFSET ajoute une valeur constante pour s'assurer que même les lectures les plus basses donnent une valeur positive.

Vous pouvez ajuster ces valeurs selon vos besoins. Par exemple, si vous obtenez généralement des valeurs autour de 7 et que vous voulez qu'elles soient plus élevées, vous pourriez augmenter SCALE_FACTOR à 2 ou 3.

Conversion en NTU :
Si vous souhaitez convertir ces valeurs en une unité standard comme le NTU, vous devrez calibrer votre capteur avec des solutions de turbidité connues et créer une courbe de calibration. Cela nécessiterait des mesures avec des échantillons de référence et l'établissement d'une fonction de conversion.
Limitations :
Gardez à l'esprit que l'ajustement logiciel ne peut pas augmenter la sensibilité physique du capteur. Si le capteur ne détecte pas de différence entre deux échantillons, l'ajustement logiciel ne créera pas cette différence.

N'hésitez pas à ajuster les valeurs de SCALE_FACTOR et OFFSET pour obtenir une plage de lecture qui convient à vos besoins spécifiques.
*/