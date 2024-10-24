/*
 * OxygenSensor.cpp
 * This file provides the implementation of the OxygenSensor class defined in OxygenSensor.h.
 * The class reads dissolved oxygen values from a dissolved oxygen sensor and performs calibration.
 */

#include "OxygenSensor.h"

// Table APHA pour DO saturation (15-40°C)
const float OxygenSensor::DO_TABLE[26] = {
    10.08,  // 15°C
    9.86,   // 16°C
    9.66,   // 17°C
    9.47,   // 18°C
    9.28,   // 19°C
    9.09,   // 20°C
    8.91,   // 21°C
    8.73,   // 22°C
    8.56,   // 23°C
    8.38,   // 24°C
    8.22,   // 25°C
    8.07,   // 26°C
    7.92,   // 27°C
    7.77,   // 28°C
    7.63,   // 29°C
    7.49,   // 30°C
    7.36,   // 31°C
    7.23,   // 32°C
    7.10,   // 33°C
    6.98,   // 34°C
    6.86,   // 35°C
    6.75,   // 36°C
    6.64,   // 37°C
    6.53,   // 38°C
    6.43,   // 39°C
    6.33    // 40°C
};

OxygenSensor::OxygenSensor(int pin, const char* name) 
    : _pin(pin), _name(name), calibrationState(CalibrationState::NONE) {
    resetCalibration();
}

void OxygenSensor::begin() {
    loadCalibrationFromEEPROM();
    Serial.print(_name);
    Serial.println(F(" initialized"));
}

float OxygenSensor::readValue(float temperature) {
    // Protection température
    if(temperature < 15.0f || temperature > 40.0f) {
        Serial.print(F("Warning: Temperature out of range (15-40°C): "));
        Serial.println(temperature);
        temperature = constrain(temperature, 15.0f, 40.0f);
    }

    // Lecture tension avec moyenne mobile
    float voltage = readAverageVoltage();
    
    // Calcul DO selon état calibration
    float doValue = 0.0f;
    if (calibrationState == CalibrationState::COMPLETE) {
        doValue = calculateDO(voltage, temperature);
    } else {
        Serial.println(F("Warning: Incomplete calibration"));
        doValue = calculateUncalibratedDO(voltage, temperature);
    }

    // Debug détaillé
    printDebugInfo(voltage, temperature, doValue);
    
    return doValue;
}

void OxygenSensor::saveZeroPoint(float temp) {
    zeroVoltage = readAverageVoltage();
    zeroTemperature = temp;
    updateCalibrationState();
    saveCalibrationToEEPROM();
    
    Serial.print(F("Zero point saved: "));
    Serial.print(zeroVoltage, 1);
    Serial.print(F("mV at "));
    Serial.print(temp, 1);
    Serial.println(F("°C"));
}

void OxygenSensor::saveSaturationLowTemp(float temp) {
    saturationVoltageLow = readAverageVoltage();
    saturationTempLow = temp;
    updateCalibrationState();
    saveCalibrationToEEPROM();
    
    Serial.print(F("Low temp saturation saved: "));
    Serial.print(saturationVoltageLow, 1);
    Serial.print(F("mV at "));
    Serial.print(temp, 1);
    Serial.println(F("°C"));
}

void OxygenSensor::saveSaturationHighTemp(float temp) {
    saturationVoltageHigh = readAverageVoltage();
    saturationTempHigh = temp;
    updateCalibrationState();
    saveCalibrationToEEPROM();
    
    Serial.print(F("High temp saturation saved: "));
    Serial.print(saturationVoltageHigh, 1);
    Serial.print(F("mV at "));
    Serial.print(temp, 1);
    Serial.println(F("°C"));
}

void OxygenSensor::resetCalibration() {
    zeroVoltage = 0.0f;
    saturationVoltageLow = 0.0f;
    saturationVoltageHigh = 0.0f;
    zeroTemperature = 20.0f;
    saturationTempLow = 20.0f;
    saturationTempHigh = 35.0f;
    calibrationState = CalibrationState::NONE;
    saveCalibrationToEEPROM();
    Serial.println(F("Calibration reset"));
}

String OxygenSensor::getCalibrationStatus() {
    String status = F("O2 Calibration Status:\n");
    
    // Point zéro
    status += F("Zero: ");
    status += String(zeroVoltage, 1);
    status += F("mV at ");
    status += String(zeroTemperature, 1);
    status += F("C\n");
    
    // Point saturation basse température
    status += F("Sat Low: ");
    status += String(saturationVoltageLow, 1);
    status += F("mV at ");
    status += String(saturationTempLow, 1);
    status += F("C\n");
    
    // Point saturation haute température
    status += F("Sat High: ");
    status += String(saturationVoltageHigh, 1);
    status += F("mV at ");
    status += String(saturationTempHigh, 1);
    status += F("C\n");
    
    // État
    status += F("State: ");
    switch (calibrationState) {
        case CalibrationState::NONE: 
            status += F("Not calibrated"); 
            break;
        case CalibrationState::PARTIAL: 
            status += F("Partial (need more points)"); 
            break;
        case CalibrationState::COMPLETE: 
            status += F("Complete"); 
            break;
    }
    return status;
}

float OxygenSensor::readAverageVoltage() {
    uint32_t sum = 0;
    for(int i = 0; i < SAMPLES_COUNT; i++) {
        sum += analogRead(_pin);
        delay(10);  // Court délai entre les lectures
    }
    return (sum * VREF) / (ADC_RES * SAMPLES_COUNT);
}

float OxygenSensor::calculateDO(float voltage, float temperature) {
    // 1. Obtenir concentration à saturation pour cette température
    float saturationDO = getSaturationDO(temperature);
    
    // 2. Obtenir tension de saturation interpolée
    float saturationVoltage = interpolateVoltage(temperature);
    
    // 3. Calculer DO avec équation de Nernst modifiée
    float voltageRange = saturationVoltage - zeroVoltage;
    
    // Protection division par zéro et validations
    if (voltageRange < 50.0f) {  // Moins de 50mV entre zéro et saturation
        Serial.println(F("Error: Invalid voltage range"));
        return 0.0f;
    }
    
    float doValue = saturationDO * (voltage - zeroVoltage) / voltageRange;
    
    // Ne jamais retourner de valeur négative ou au-dessus de la saturation
    return constrain(doValue, 0.0f, saturationDO);
}

float OxygenSensor::calculateUncalibratedDO(float voltage, float temperature) {
    float saturationDO = getSaturationDO(temperature);
    return saturationDO * voltage / 1000.0f;  // Approximation grossière
}

float OxygenSensor::getSaturationDO(float temperature) {
    uint8_t index = constrain((int)(temperature - TEMP_MIN), 0, TEMP_MAX - TEMP_MIN);
    return DO_TABLE[index];
}

float OxygenSensor::interpolateVoltage(float temperature) {
    if (temperature <= saturationTempLow) {
        return saturationVoltageLow;
    }
    if (temperature >= saturationTempHigh) {
        return saturationVoltageHigh;
    }
    
    float ratio = (temperature - saturationTempLow) / (saturationTempHigh - saturationTempLow);
    return saturationVoltageLow + ratio * (saturationVoltageHigh - saturationVoltageLow);
}

void OxygenSensor::updateCalibrationState() {
    if (zeroVoltage > 0) {
        if (saturationVoltageLow > 0 && saturationVoltageHigh > 0) {
            calibrationState = CalibrationState::COMPLETE;
        } else if (saturationVoltageLow > 0 || saturationVoltageHigh > 0) {
            calibrationState = CalibrationState::PARTIAL;
        }
    }
}

void OxygenSensor::printDebugInfo(float voltage, float temperature, float doValue) {
    Serial.print(F("O2: V="));
    Serial.print(voltage, 1);
    Serial.print(F("mV T="));
    Serial.print(temperature, 1);
    Serial.print(F("°C DO="));
    Serial.print(doValue, 2);
    Serial.print(F("mg/L Sat="));
    Serial.print(getSaturationDO(temperature), 2);
    Serial.print(F("mg/L VsatT="));
    Serial.print(interpolateVoltage(temperature), 1);
    Serial.println(F("mV"));
}

void OxygenSensor::saveCalibrationToEEPROM() {
    int addr = O2_EEPROM_ADDR;
    EEPROM.put(addr, zeroVoltage);
    addr += sizeof(float);
    EEPROM.put(addr, saturationVoltageLow);
    addr += sizeof(float);
    EEPROM.put(addr, saturationVoltageHigh);
    addr += sizeof(float);
    EEPROM.put(addr, zeroTemperature);
    addr += sizeof(float);
    EEPROM.put(addr, saturationTempLow);
    addr += sizeof(float);
    EEPROM.put(addr, saturationTempHigh);
    addr += sizeof(float);
    EEPROM.put(addr, (int)calibrationState);
}

void OxygenSensor::loadCalibrationFromEEPROM() {
    int addr = O2_EEPROM_ADDR;
    EEPROM.get(addr, zeroVoltage);
    addr += sizeof(float);
    EEPROM.get(addr, saturationVoltageLow);
    addr += sizeof(float);
    EEPROM.get(addr, saturationVoltageHigh);
    addr += sizeof(float);
    EEPROM.get(addr, zeroTemperature);
    addr += sizeof(float);
    EEPROM.get(addr, saturationTempLow);
    addr += sizeof(float);
    EEPROM.get(addr, saturationTempHigh);
    addr += sizeof(float);
    int state;
    EEPROM.get(addr, state);
    calibrationState = (CalibrationState)state;
}