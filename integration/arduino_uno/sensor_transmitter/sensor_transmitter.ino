#include <SoftwareSerial.h>
#include "SensorController.h"
#include "PHSensor.h"
#include "OxygenSensor.h"
#include <EEPROM.h>

SoftwareSerial teensySerial(2, 3); // RX, TX

PHSensor phSensor(A1, "phSensor");
OxygenSensor oxygenSensor(A5, "oxygenSensor");

SensorController sensorController;

void initializeEEPROM() {
    byte value;
    EEPROM.get(0, value);
    if (value == 255) { // Blank EEPROM
        // Marquer l'EEPROM comme initialisée
        EEPROM.put(0, (byte)0);
        // Les valeurs par défaut sont maintenant gérées dans le constructeur de OxygenSensor
        Serial.println(F("EEPROM initialized"));
    }
}

void setup() {
    Serial.begin(115200);
    teensySerial.begin(9600);

    // Initialisation des capteurs
    sensorController.initialize(phSensor, oxygenSensor);
    SensorController::beginAll();

    initializeEEPROM();
    
    Serial.println(F("System initialized"));
    // Afficher l'état initial des calibrations
    Serial.println(oxygenSensor.getCalibrationStatus());
}

void loop() {
    if (teensySerial.available()) {
        String command = teensySerial.readStringUntil('\n');
        command.trim();  // Enlever espaces et retours chariot
        
        if (command.startsWith(F("PH:READ:"))) {
            float temp = command.substring(8).toFloat();
            float ph = phSensor.readValue(temp);
            teensySerial.print(F("PH:"));
            teensySerial.println(ph);
        }
        else if (command.startsWith(F("O2:"))) {  // Ajout du préfixe O2:
            if (command.startsWith(F("O2:READ:"))) {
                float temp = command.substring(8).toFloat();
                float o2 = oxygenSensor.readValue(temp);
                teensySerial.print(F("O2:"));
                teensySerial.println(o2);
            }
            else if (command.startsWith(F("O2:CAL:"))) {
                String calCommand = command.substring(7);  // Enlever "O2:CAL:"
                
                if (calCommand.startsWith(F("ZERO:"))) {
                    float temp = calCommand.substring(5).toFloat();
                    oxygenSensor.saveZeroPoint(temp);
                    teensySerial.println(F("O2:CAL:ZERO_OK"));
                }
                else if (calCommand.startsWith(F("SAT_LOW:"))) {
                    float temp = calCommand.substring(8).toFloat();
                    oxygenSensor.saveSaturationLowTemp(temp);
                    teensySerial.println(F("O2:CAL:SAT_LOW_OK"));
                }
                else if (calCommand.startsWith(F("SAT_HIGH:"))) {
                    float temp = calCommand.substring(9).toFloat();
                    oxygenSensor.saveSaturationHighTemp(temp);
                    teensySerial.println(F("O2:CAL:SAT_HIGH_OK"));
                }
                else if (calCommand == F("STATUS")) {
                    String status = oxygenSensor.getCalibrationStatus();
                    teensySerial.println("O2:CAL:STATUS:" + status);
                }
                else if (calCommand == F("RESET")) {
                    oxygenSensor.resetCalibration();
                    teensySerial.println(F("O2:CAL:RESET_OK"));
                }
            }
        }

        // Debug sur le port série
        Serial.print(F("Command received: "));
        Serial.println(command);
    }
}