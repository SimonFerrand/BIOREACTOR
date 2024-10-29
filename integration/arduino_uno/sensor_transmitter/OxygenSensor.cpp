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
}

void OxygenSensor::begin() {
    delay(100);
    
    EEPROM.get(EEPROM_START_ADDR, calibData);
    
    if(calibData.isInitialized != 0xAB) {
        Serial.println(F("First use - Initializing"));
        resetCalibration();
    } else {
        updateCalibrationState();
    }
    
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

    float voltage = readAverageVoltage();
    
    float doValue = 0.0f;
    if (calibrationState == CalibrationState::COMPLETE) {
        doValue = calculateDO(voltage, temperature);
    } else {
        Serial.println(F("Warning: Incomplete calibration"));
        doValue = calculateUncalibratedDO(voltage, temperature);
    }

    doValue = round(doValue * 10.0f) / 10.0f;  // Rounded to 1 decimal place
    
    printDebugInfo(voltage, temperature, doValue);
    
    return doValue;
}

void OxygenSensor::saveZeroPoint(float temp) {
    float voltage = readAverageVoltage();
    
    calibData.zeroVoltage = voltage;
    calibData.zeroTemperature = temp;
    calibData.isInitialized = 0xAB;
    
    EEPROM.put(EEPROM_START_ADDR, calibData);
    updateCalibrationState();
    
    Serial.print(F("Zero point saved: "));
    Serial.print(voltage, 1);
    Serial.print(F("mV at "));
    Serial.print(temp, 1);
    Serial.println(F("°C"));
}


void OxygenSensor::saveSaturationLowTemp(float temp) {
    float voltage = readAverageVoltage();
    
    calibData.saturationVoltageLow = voltage;
    calibData.saturationTempLow = temp;
    
    EEPROM.put(EEPROM_START_ADDR, calibData);
    updateCalibrationState();
    
    Serial.print(F("Low temp saturation saved: "));
    Serial.print(voltage, 1);
    Serial.print(F("mV at "));
    Serial.print(temp, 1);
    Serial.println(F("°C"));
}

void OxygenSensor::saveSaturationHighTemp(float temp) {
    float voltage = readAverageVoltage();
    
    calibData.saturationVoltageHigh = voltage;
    calibData.saturationTempHigh = temp;
    
    EEPROM.put(EEPROM_START_ADDR, calibData);
    updateCalibrationState();
    
    Serial.print(F("High temp saturation saved: "));
    Serial.print(voltage, 1);
    Serial.print(F("mV at "));
    Serial.print(temp, 1);
    Serial.println(F("°C"));
}


String OxygenSensor::getCalibrationStatus() {
    String status = F("O2 Calibration Status:\n");
    
    status += F("Zero: ");
    status += String(calibData.zeroVoltage, 1);
    status += F("mV at ");
    status += String(calibData.zeroTemperature, 1);
    status += F("C\n");
    
    status += F("Sat Low: ");
    status += String(calibData.saturationVoltageLow, 1);
    status += F("mV at ");
    status += String(calibData.saturationTempLow, 1);
    status += F("C\n");
    
    status += F("Sat High: ");
    status += String(calibData.saturationVoltageHigh, 1);
    status += F("mV at ");
    status += String(calibData.saturationTempHigh, 1);
    status += F("C\n");
    
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
        delay(10);  
    }
    return (sum * VREF) / (ADC_RES * SAMPLES_COUNT);
}

float OxygenSensor::calculateDO(float voltage, float temperature) {
    // 1. get saturation concentration for this temperature
    float saturationDO = getSaturationDO(temperature);
    // 2. get interpolated saturation voltage
    float saturationVoltage = interpolateVoltage(temperature);
    // 3. calculate DO with modified Nernst equation
    float voltageRange = saturationVoltage - calibData.zeroVoltage;
    
    // Zero division protection and validation
    if (voltageRange < 50.0f) {  // Moins de 50mV entre zéro et saturation
        Serial.println(F("Error: Invalid voltage range"));
        return 0.0f;
    }
    
    float doValue = saturationDO * (voltage - calibData.zeroVoltage) / voltageRange;

    // Never return a negative value or a value above saturation
    return constrain(doValue, 0.0f, saturationDO);
}

float OxygenSensor::calculateUncalibratedDO(float voltage, float temperature) {
    float saturationDO = getSaturationDO(temperature);
    return saturationDO * voltage / 1000.0f;  // Rough approximation
}

float OxygenSensor::getSaturationDO(float temperature) {
    uint8_t index = constrain((int)(temperature - TEMP_MIN), 0, TEMP_MAX - TEMP_MIN);
    return DO_TABLE[index];
}


float OxygenSensor::interpolateVoltage(float temperature) {
    if (temperature <= calibData.saturationTempLow) {
        return calibData.saturationVoltageLow;
    }
    if (temperature >= calibData.saturationTempHigh) {
        return calibData.saturationVoltageHigh;
    }
    
    float ratio = (temperature - calibData.saturationTempLow) / 
                  (calibData.saturationTempHigh - calibData.saturationTempLow);
    return calibData.saturationVoltageLow + 
           ratio * (calibData.saturationVoltageHigh - calibData.saturationVoltageLow);
}

void OxygenSensor::updateCalibrationState() {
    if (calibData.zeroVoltage > 0) {
        if (calibData.saturationVoltageLow > 0 && calibData.saturationVoltageHigh > 0) {
            calibrationState = CalibrationState::COMPLETE;
        } else if (calibData.saturationVoltageLow > 0 || calibData.saturationVoltageHigh > 0) {
            calibrationState = CalibrationState::PARTIAL;
        } else {
            calibrationState = CalibrationState::NONE;
        }
    } else {
        calibrationState = CalibrationState::NONE;
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


void OxygenSensor::loadCalibration() {
    Serial.println(F("\n=== EEPROM Raw Values ==="));
    
    // Lire les données directement dans calibData
    EEPROM.get(EEPROM_START_ADDR, calibData);
    
    Serial.print(F("Zero V: ")); Serial.println(calibData.zeroVoltage);
    Serial.print(F("Zero T: ")); Serial.println(calibData.zeroTemperature);
    Serial.print(F("Sat Low V: ")); Serial.println(calibData.saturationVoltageLow);
    Serial.print(F("Sat Low T: ")); Serial.println(calibData.saturationTempLow);
    Serial.print(F("Sat High V: ")); Serial.println(calibData.saturationVoltageHigh);
    Serial.print(F("Sat High T: ")); Serial.println(calibData.saturationTempHigh);

    // Validation
    if (!isValidValues()) {
        Serial.println(F("Invalid values detected - Resetting to defaults"));
        resetToDefaultValues();
    } else {
        updateCalibrationState();
        Serial.println(F("Calibration loaded successfully"));
    }
}

void OxygenSensor::resetToDefaultValues() {
    // Reset calibData to default values
    calibData.zeroVoltage = 0.0f;
    calibData.zeroTemperature = 25.0f;
    calibData.saturationVoltageLow = 0.0f;
    calibData.saturationTempLow = 25.0f;
    calibData.saturationVoltageHigh = 0.0f;
    calibData.saturationTempHigh = 25.0f;
    calibData.isInitialized = 0xAB;
    
    // Save at once
    EEPROM.put(EEPROM_START_ADDR, calibData);
    
    calibrationState = CalibrationState::NONE;
    Serial.println(F("Default values set"));
}

void OxygenSensor::resetCalibration() {
    calibData.zeroVoltage = 0.0f;
    calibData.zeroTemperature = 25.0f;
    calibData.saturationVoltageLow = 0.0f;
    calibData.saturationTempLow = 25.0f;
    calibData.saturationVoltageHigh = 0.0f;
    calibData.saturationTempHigh = 25.0f;
    calibData.isInitialized = 0xAB;
    
    EEPROM.put(EEPROM_START_ADDR, calibData);
    updateCalibrationState();
    
    Serial.println(F("Calibration reset complete"));
}


bool OxygenSensor::isValidValues() {
    if (calibData.zeroVoltage < 0.0f || calibData.zeroVoltage > 3000.0f) return false;
    if (calibData.saturationVoltageLow > 3000.0f) return false;
    if (calibData.saturationVoltageHigh > 3000.0f) return false;
    
    if (calibData.zeroTemperature < 0.0f || calibData.zeroTemperature > 40.0f) return false;
    if (calibData.saturationTempLow < 0.0f || calibData.saturationTempLow > 40.0f) return false;
    if (calibData.saturationTempHigh < 0.0f || calibData.saturationTempHigh > 40.0f) return false;
    
    return true;
}

void OxygenSensor::debugEEPROM() {
    Serial.println(F("\n=== EEPROM Debug ==="));
    Serial.println(F("Current calibration data:"));
    Serial.print(F("Zero: ")); 
    Serial.print(calibData.zeroVoltage);
    Serial.print(F("mV at "));
    Serial.print(calibData.zeroTemperature);
    Serial.println(F("°C"));
    
    Serial.print(F("Low: "));
    Serial.print(calibData.saturationVoltageLow);
    Serial.print(F("mV at "));
    Serial.print(calibData.saturationTempLow);
    Serial.println(F("°C"));
    
    Serial.print(F("High: "));
    Serial.print(calibData.saturationVoltageHigh);
    Serial.print(F("mV at "));
    Serial.print(calibData.saturationTempHigh);
    Serial.println(F("°C"));
    
    Serial.print(F("Initialized flag: 0x"));
    Serial.println(calibData.isInitialized, HEX);
}


