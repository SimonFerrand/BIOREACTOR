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
        // Initialize with default values
        // For oxygen sensor
        int defaultCalibrationPoints = 0;
        EEPROM.put(O2_EEPROM_ADDR, defaultCalibrationPoints);
        // For pH meter, if required
        // ...
        
        Serial.println(F("EEPROM initialized with default values"));
    }
}

void setup() {
    Serial.begin(115200);
    teensySerial.begin(9600);
    sensorController.initialize(phSensor, oxygenSensor);
    SensorController::beginAll();

    initializeEEPROM();
}

void loop() {
    if (teensySerial.available()) {
        String command = teensySerial.readStringUntil('\n');
        
        if (command.startsWith(F("PH:READ:"))) {
            float temp = command.substring(8).toFloat();
            float ph = phSensor.readValue(temp);
            teensySerial.print(F("PH:"));
            teensySerial.println(ph);
        }
        else if (command.startsWith(F("PH:CAL:"))) {
            int firstColon = command.indexOf(':', 7);
            int lastColon = command.lastIndexOf(':');
            if (firstColon != -1 && lastColon != -1) {
                String calCommand = command.substring(7, firstColon);
                float temp = command.substring(lastColon + 1).toFloat();
                String result = phSensor.calibration(calCommand.c_str(), temp);
                teensySerial.println("PH:CAL:" + result);
            } else {
                teensySerial.println(F("PH:CAL:ERROR:Invalid command format"));
            }
        }
        else if (command.startsWith(F("O2:READ:"))) {
            float temp = command.substring(8).toFloat();
            float o2 = oxygenSensor.readValue(temp);
            teensySerial.print(F("O2:"));
            teensySerial.println(o2);
        }
        else if (command.startsWith(F("O2:CAL:START:"))) {
            int points = command.substring(13).toInt();
            if (points > 0 && points <= 3) {
                oxygenSensor.startCalibration(points);
                teensySerial.println(F("O2:CAL:STARTED"));
            } else {
                teensySerial.println(F("O2:CAL:ERROR:Invalid number of points"));
            }
        }
        else if (command.startsWith(F("O2:CAL:SAVE:"))) {
            float temp = command.substring(12).toFloat();
            oxygenSensor.saveCalibrationPoint(temp);
            teensySerial.println(F("O2:CAL:POINT_SAVED"));
        }
        else if (command == F("O2:CAL:FINISH")) {
            oxygenSensor.finishCalibration();
            teensySerial.println(F("O2:CAL:COMPLETED"));
        }
        else if (command == F("O2:CAL:STATUS")) {
            String status = oxygenSensor.getCalibrationStatus();
            teensySerial.print(F("O2:CAL:STATUS:"));
            teensySerial.println(status);
        }
        else if (command == F("O2:CAL:RESET")) {
            oxygenSensor.resetCalibration();
            teensySerial.println(F("O2:CAL:RESET_DONE"));
        }
    }
}