/*
 * PHSensor.cpp
 * This file provides the implementation of the PHSensor class defined in PHSensor.h.
 * The class reads pH value from a pH sensor and performs calibration.
 */

#include "PHSensor.h"

// Constructor for PHSensor
PHSensor::PHSensor(int pin, const char* name)
    : _pin(pin), _name(name), _voltage(0), temperature(0) {}

// Method to initialize the pH sensor
void PHSensor::begin() {
    _ph.begin();
    Serial.print(_name);
    Serial.println(F(" initialized"));
    displayCalibrationValues();
}

void PHSensor::displayCalibrationValues() {
    float neutralVoltage, acidVoltage;
    EEPROM.get(PH_EEPROM_ADDR, neutralVoltage);
    EEPROM.get(PH_EEPROM_ADDR + 4, acidVoltage);
    
    Serial.print(_name);
    Serial.println(F(" - Stored calibration values:"));
    Serial.print(F("  Neutral (pH 7.0) voltage: "));
    Serial.print(neutralVoltage);
    Serial.println(F(" mV"));
    Serial.print(F("  Acid (pH 4.0) voltage: "));
    Serial.print(acidVoltage);
    Serial.println(F(" mV"));
    
    float slope = (acidVoltage - neutralVoltage) / (7.0 - 4.0); // library
    //float slope = (neutralVoltage - acidVoltage) / (4.0 - 7.0);
    Serial.print(F("  Calculated slope: "));
    Serial.print(slope);
    Serial.println(F(" mV/pH"));
    
    if (acidVoltage > neutralVoltage) {
        Serial.println(F("ATTENTION: The calibration values seem to be inverted!"));
    }
}

float PHSensor::readValue(float temperature) {
    _voltage = analogRead(_pin) / 1024.0 * 5000; // Convert analog reading to millivolts / 5000 for 5.0V and 3300 for 3.3V
    float phValue = _ph.readPH(_voltage, temperature);
    phValue = round(phValue * 10) / 10.0; // Round to 1 decimal place
    Serial.print(_name);
    Serial.print(F(" - Tension: "));
    Serial.print(_voltage);
    Serial.print(F(" mV, Température: "));
    Serial.print(temperature);
    Serial.print(F("°C, pH: "));
    Serial.println(phValue, 1); // digit after the decimal point
    return phValue;
}

// Method to handle pH calibration commands
String PHSensor::calibration(const char* cmd, float temperature) {
    _voltage = analogRead(_pin) / 1024.0 * 5000; // Convert analog reading to millivolts
    String result;

    if (strcmp(cmd, "ENTERPH") == 0 || strcmp(cmd, "CALPH") == 0 || strcmp(cmd, "EXITPH") == 0) {
        _ph.calibration(_voltage, temperature, const_cast<char*>(cmd)); // Call the calibration method from DFRobot_PH class
        result = String(cmd) + "_OK";
    } else {
        result = "INVALID_CMD";
    }

    Serial.print(_name);
    Serial.print(F(" - Calibration effectuée. Cmd: "));
    Serial.print(cmd);
    Serial.print(F(", Tension: "));
    Serial.print(_voltage);
    Serial.print(F(" mV, Température: "));
    Serial.print(temperature);
    Serial.println(F("°C"));

    return result;
}