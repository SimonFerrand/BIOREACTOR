/*
 * OxygenSensor.cpp
 * This file provides the implementation of the OxygenSensor class defined in OxygenSensor.h.
 * The class reads dissolved oxygen values from a dissolved oxygen sensor and performs calibration.
 */

#include "OxygenSensor.h"

const uint16_t OxygenSensor::DO_Table[41] = {
    14460, 14220, 13820, 13440, 13090, 12740, 12420, 12110, 11810, 11530,
    11260, 11010, 10770, 10530, 10300, 10080, 9860, 9660, 9460, 9270,
    9080, 8900, 8730, 8570, 8410, 8250, 8110, 7960, 7820, 7690,
    7560, 7430, 7300, 7180, 7070, 6950, 6840, 6730, 6630, 6530, 6410
};

OxygenSensor::OxygenSensor(int pin, const char* name)
    : _pin(pin), _name(name), calibrationState(CalibrationState::NONE) {
}

void OxygenSensor::begin() {
    loadCalibrationFromEEPROM();
    Serial.print(_name);
    Serial.println(F(" initialized"));
}

float OxygenSensor::readValue(float temperature) {
    uint16_t rawValue = analogRead(_pin);
    uint32_t voltage = uint32_t(VREF) * rawValue / ADC_RES;

    uint32_t V_saturation;
    if (calibrationPoints == 1) {
        V_saturation = (uint32_t)calibrationVoltages[0] + (uint32_t)35 * temperature - (uint32_t)calibrationTemperatures[0] * 35;
    } else if (calibrationPoints >= 2) {
        int lowerIndex = 0, upperIndex = 1;
        for (int i = 1; i < calibrationPoints; i++) {
            if (calibrationTemperatures[i] <= temperature && calibrationTemperatures[i] > calibrationTemperatures[lowerIndex]) {
                lowerIndex = i;
            }
            if (calibrationTemperatures[i] > temperature && calibrationTemperatures[i] < calibrationTemperatures[upperIndex]) {
                upperIndex = i;
            }
        }
        
        V_saturation = (int16_t)((int8_t)temperature - calibrationTemperatures[upperIndex]) * 
                       ((uint32_t)calibrationVoltages[lowerIndex] - calibrationVoltages[upperIndex]) / 
                       ((uint8_t)calibrationTemperatures[lowerIndex] - calibrationTemperatures[upperIndex]) + 
                       calibrationVoltages[upperIndex];
    } else {
        V_saturation = 1127.6 * pow(0.978, temperature);  // Default if not calibrated
    }

    float oxygenValue = (voltage * DO_Table[(int)temperature] / V_saturation); // DO in µg/L
    oxygenValue = round((oxygenValue / 1000) * 10) / 10.0; // Convert to g/L and round to 1 decimal place
    Serial.print(F("oxygenSensor - Tension: "));
    Serial.print(voltage);
    Serial.print(F(" mV, Température: "));
    Serial.print(temperature);
    Serial.print(F("°C, O2: "));
    Serial.print(oxygenValue, 1); // digit after the decimal point
    Serial.println(F(" mg/L"));
    return oxygenValue; 
}

void OxygenSensor::startCalibration(int points) {
    if (points > 0 && points <= MAX_CALIBRATION_POINTS) {
        calibrationPoints = points;
        currentCalibrationPoint = 0;
        calibrationState = CalibrationState::IN_PROGRESS;
        Serial.print(F("O2 calibration started for "));
        Serial.print(points);
        Serial.println(F(" point(s)"));
    } else {
        Serial.println(F("Invalid number of calibration points"));
        Serial.println(F("O2:CAL:ERROR:Invalid number of points"));
    }
}

void OxygenSensor::saveCalibrationPoint(float temperature) {
    if (calibrationState == CalibrationState::IN_PROGRESS) {
        if (currentCalibrationPoint < calibrationPoints) {
            uint16_t rawValue = analogRead(_pin);
            uint16_t voltage = rawValue * (VREF / ADC_RES);
            
            calibrationVoltages[currentCalibrationPoint] = voltage;
            calibrationTemperatures[currentCalibrationPoint] = temperature;
            
            currentCalibrationPoint++;
            
            Serial.print(F("O2 calibration point "));
            Serial.print(currentCalibrationPoint);
            Serial.print(F(" saved: V="));
            Serial.print(voltage);
            Serial.print(F("mV, T="));
            Serial.print(temperature);
            Serial.println(F("°C"));
            
            if (currentCalibrationPoint == calibrationPoints) {
                Serial.println(F("All O2 calibration points saved. Use 'o2 calibrate finish' to complete."));
            }
        } else {
            Serial.println(F("All O2 calibration points already saved. Use 'o2 calibrate finish' to complete."));
        }
    } else {
        Serial.println(F("O2 calibration not started. Use 'o2 calibrate start <points>' first."));
    }
}

void OxygenSensor::finishCalibration() {
    if (calibrationState == CalibrationState::IN_PROGRESS && currentCalibrationPoint == calibrationPoints) {
        calibrationState = CalibrationState::COMPLETED;
        saveCalibrationToEEPROM();
        Serial.println(F("O2 calibration completed and saved."));
    } else {
        Serial.println(F("Cannot finish O2 calibration. Not all points are saved."));
    }
}

String OxygenSensor::getCalibrationStatus() const {
    String status = F("O2 Calibration Status: ");
    switch (calibrationState) {
        case CalibrationState::NONE:
            status += F("Not calibrated");
            break;
        case CalibrationState::IN_PROGRESS:
            status += String(currentCalibrationPoint);
            status += F(" of ");
            status += String(calibrationPoints);
            status += F(" points saved");
            break;
        case CalibrationState::COMPLETED:
            status += F("Completed with ");
            status += String(calibrationPoints);
            status += F(" point(s)");
            break;
    }
    return status;
}
void OxygenSensor::resetCalibration() {
    calibrationPoints = 0;
    currentCalibrationPoint = 0;
    calibrationState = CalibrationState::NONE;
    for (int i = 0; i < MAX_CALIBRATION_POINTS; i++) {
        calibrationVoltages[i] = 0;
        calibrationTemperatures[i] = 0;
    }
    saveCalibrationToEEPROM();
    Serial.println(F("O2 calibration reset"));
}

void OxygenSensor::saveCalibrationToEEPROM() {
    int addr = O2_EEPROM_ADDR;
    EEPROM.put(addr, calibrationPoints);
    addr += sizeof(int);
    
    for (int i = 0; i < calibrationPoints; i++) {
        EEPROM.put(addr, calibrationVoltages[i]);
        addr += sizeof(uint16_t);
        EEPROM.put(addr, calibrationTemperatures[i]);
        addr += sizeof(uint8_t);
    }
    
    Serial.println(F("O2 calibration saved to EEPROM"));
}

void OxygenSensor::loadCalibrationFromEEPROM() {
    int addr = O2_EEPROM_ADDR;
    EEPROM.get(addr, calibrationPoints);
    addr += sizeof(int);
    
    Serial.print(F("Loaded calibrationPoints: "));
    Serial.println(calibrationPoints);
    
    if (calibrationPoints > 0 && calibrationPoints <= MAX_CALIBRATION_POINTS) {
        bool validData = true;
        for (int i = 0; i < calibrationPoints; i++) {
            EEPROM.get(addr, calibrationVoltages[i]);
            addr += sizeof(uint16_t);
            EEPROM.get(addr, calibrationTemperatures[i]);
            addr += sizeof(uint8_t);
            
            if (calibrationVoltages[i] == 0 || calibrationVoltages[i] > 5000 || 
                calibrationTemperatures[i] > 100) {
                validData = false;
                break;
            }
            
            Serial.print(F("Loaded O2 calibration point "));
            Serial.print(i);
            Serial.print(F(": Voltage="));
            Serial.print(calibrationVoltages[i]);
            Serial.print(F(", Temp="));
            Serial.println(calibrationTemperatures[i]);
        }
        
        if (validData) {
            calibrationState = CalibrationState::COMPLETED;
            Serial.println(F("O2 calibration loaded from EEPROM"));
        } else {
            Serial.println(F("Invalid O2 calibration data in EEPROM"));
            resetCalibration();
        }
    } else {
        Serial.print(F("Invalid O2 calibrationPoints: "));
        Serial.println(calibrationPoints);
        resetCalibration();
    }
}