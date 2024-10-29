#include <SoftwareSerial.h>
#include "SensorController.h"
#include "PHSensor.h"
#include "OxygenSensor.h"
#include <EEPROM.h>

// Configuration communication
SoftwareSerial teensySerial(2, 3); // RX, TX

// Capteurs
PHSensor phSensor(A1, "phSensor");
OxygenSensor oxygenSensor(A5, "oxygenSensor");

void setup() {
    // Initialisation des communications
    Serial.begin(115200);
    while(!Serial) { }  // Attendre que le port série soit prêt
    delay(1000);
    teensySerial.begin(9600);
    
    Serial.println(F("=== System Initialization ==="));
    
    // Initialiser le controller et les capteurs
    SensorController::initialize(phSensor, oxygenSensor);
    SensorController::beginAll();
    
    // État initial
    Serial.println(F("\nInitial calibration status:"));
    Serial.println(oxygenSensor.getCalibrationStatus());
    
    Serial.println(F("\nSystem ready"));
}

void loop() {
    if (teensySerial.available()) {
        String command = teensySerial.readStringUntil('\n');
        command.trim();
        
        Serial.print(F("Command received: "));
        Serial.println(command);

        // pH commands
        if (command.startsWith(F("PH:READ:"))) {
            float temp = command.substring(8).toFloat();
            float ph = SensorController::readSensor("phSensor", temp);
            teensySerial.print(F("PH:"));
            teensySerial.println(ph);
        }
        else if (command.startsWith(F("PH:CAL:ENTERPH:"))) {
            float temp = command.substring(14).toFloat();
            String result = phSensor.calibration("ENTERPH", temp);
            teensySerial.println("ENTERPH:" + result);
        }
        else if (command.startsWith(F("PH:CAL:CALPH:"))) {
            float temp = command.substring(12).toFloat();
            String result = phSensor.calibration("CALPH", temp);
            teensySerial.println("CALPH:" + result);
        }
        else if (command.startsWith(F("PH:CAL:EXITPH:"))) {
            float temp = command.substring(13).toFloat();
            String result = phSensor.calibration("EXITPH", temp);
            teensySerial.println("EXITPH:" + result);
        }

        // O2 commands
        else if (command.startsWith(F("O2:READ:"))) {
            float temp = command.substring(8).toFloat();
            float o2 = SensorController::readSensor("oxygenSensor", temp);
            teensySerial.print(F("O2:"));
            teensySerial.println(o2);
        }
        else if (command == F("O2:CAL:START")) {
            teensySerial.println(F("O2:CAL:START:OK"));
        }
        else if (command.startsWith(F("O2:CAL:ZERO:"))) {
            float temp = command.substring(12).toFloat();
            oxygenSensor.saveZeroPoint(temp);
            teensySerial.println(F("O2:CAL:ZERO:OK"));
        }
        else if (command.startsWith(F("O2:CAL:SAT_LOW:"))) {
            float temp = command.substring(15).toFloat();
            oxygenSensor.saveSaturationLowTemp(temp);
            teensySerial.println(F("O2:CAL:SAT_LOW:OK"));
        }
        else if (command.startsWith(F("O2:CAL:SAT_HIGH:"))) {
            float temp = command.substring(16).toFloat();
            oxygenSensor.saveSaturationHighTemp(temp);
            teensySerial.println(F("O2:CAL:SAT_HIGH:OK"));
        }
        else if (command == F("O2:CAL:RESET")) {
            oxygenSensor.resetCalibration();
            teensySerial.println(F("O2:CAL:RESET:OK"));
        }
        else if (command == F("O2:CAL:STATUS")) {
            String status = oxygenSensor.getCalibrationStatus();
            teensySerial.println("O2:CAL:STATUS:" + status);
        }
        else {
            Serial.print(F("Unknown command: "));
            Serial.println(command);
        }
    }
}