/*
 * PHSensor.cpp
 * This file provides the implementation of the PHSensor class defined in PHSensor.h.
 * The class reads pH value from a pH sensor and performs calibration.
 */

#include "PHSensor.h"
#include "Logger.h"

// Constructor for PHSensor
PHSensor::PHSensor(int pin, PT100Sensor* tempSensor, const char* name) 
    : _pin(pin), _tempSensor(tempSensor), _name(name), _voltage(0), temperature(0) {}

// Method to initialize the pH sensor
void PHSensor::begin() {
    _ph.begin();
    //Logger::log(LogLevel::INFO, String(_name) + " initialized");
    Logger::log(LogLevel::INFO, String(_name) + F(" initialized"));
    displayCalibrationValues();
}

void PHSensor::displayCalibrationValues() {
    float neutralVoltage, acidVoltage;
    EEPROM.get(PH_EEPROM_ADDR , neutralVoltage);
    EEPROM.get(PH_EEPROM_ADDR +4, acidVoltage);
    
    Logger::log(LogLevel::INFO, String(_name) + F(" - Stored calibration values:"));
    Logger::log(LogLevel::INFO, "  Neutral (pH 7.0) voltage: " + String(neutralVoltage) + " mV");
    Logger::log(LogLevel::INFO, "  Acid (pH 4.0) voltage: " + String(acidVoltage) + " mV");
    
    float slope = (acidVoltage - neutralVoltage) / (7.0 - 4.0); // library
    //float slope = (neutralVoltage - acidVoltage) / (4.0 - 7.0);
    Logger::log(LogLevel::INFO, "  Calculated slope: " + String(slope) + " mV/pH");

    if (acidVoltage > neutralVoltage) {
        Logger::log(LogLevel::WARNING, F("ATTENTION: Les valeurs de calibration semblent inversées!"));
    }
}

// Method to read the pH value from the sensor
float PHSensor::readValue() {
    _voltage = analogRead(_pin) / 1024.0 * 5000; // Convert analog reading to millivolts
    float temperature = _tempSensor->readValue();
    float phValue = _ph.readPH(_voltage, temperature); // Calculate pH value with temperature compensation
    //Logger::log(LogLevel::INFO, String(_name) + F(" - Tension: ") + String(_voltage) + F(" mV, Température: ") + String(temperature) + F("°C, pH: ") + String(phValue));
    
    float manualPH = manualPHCalculation(_voltage);
    Logger::log(LogLevel::INFO, String(_name) + " - Reading: Voltage=" + 
                String(_voltage) + "mV, Temp=" + String(temperature) + 
                "°C, Library pH=" + String(phValue) + 
                ", Manual pH=" + String(manualPH));

    return phValue;
}

// Method to handle pH calibration commands
void PHSensor::calibration(const char* cmd) {
    float temperature = _tempSensor->readValue();
    _ph.calibration(_voltage, temperature, const_cast<char*>(cmd)); // Call the calibration method from DFRobot_PH class
    Logger::log(LogLevel::INFO, String(_name) + F(" - Calibration effectuée. Cmd: ") + String(cmd) + F(", Tension: ") + String(_voltage) + F(" mV, Température: ") + String(temperature) + F("°C"));
}

float PHSensor::manualPHCalculation(float voltage) {
    float neutralVoltage, acidVoltage;
    EEPROM.get(PH_EEPROM_ADDR , neutralVoltage);
    EEPROM.get(PH_EEPROM_ADDR +4, acidVoltage);
    
    //float slope = (acidVoltage - neutralVoltage) / (7.0 - 4.0);  // mV/pH    // library
    float slope = (neutralVoltage - acidVoltage) / (4.0 - 7.0);
    float calculatedPH = 7.0 + (voltage - neutralVoltage) / slope;
    
    Logger::log(LogLevel::INFO, F("Manual pH calculation:"));
    Logger::log(LogLevel::INFO, "  Input voltage: " + String(voltage) + " mV");
    Logger::log(LogLevel::INFO, "  Slope: " + String(slope) + " mV/pH");
    Logger::log(LogLevel::INFO, "  Calculated pH: " + String(calculatedPH));
    
    return calculatedPH;
}