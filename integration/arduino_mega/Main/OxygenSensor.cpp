/*
 * OxygenSensor.cpp
 * This file provides the implementation of the OxygenSensor class defined in OxygenSensor.h.
 * The class reads dissolved oxygen values from a dissolved oxygen sensor and performs calibration.
 */

#include "OxygenSensor.h"
#include "Logger.h"

const uint16_t OxygenSensor::DO_Table[41] = {
    14460, 14220, 13820, 13440, 13090, 12740, 12420, 12110, 11810, 11530,
    11260, 11010, 10770, 10530, 10300, 10080, 9860, 9660, 9460, 9270,
    9080, 8900, 8730, 8570, 8410, 8250, 8110, 7960, 7820, 7690,
    7560, 7430, 7300, 7180, 7070, 6950, 6840, 6730, 6630, 6530, 6410
};

OxygenSensor::OxygenSensor(int pin, PT100Sensor* tempSensor, const char* name)
    : _pin(pin), _tempSensor(tempSensor), _name(name), calibrationState(CalibrationState::NONE) {
    loadCalibrationFromEEPROM();
}

void OxygenSensor::begin() {
    Logger::log(LogLevel::INFO, String(_name) + F(" initialized"));
}

float OxygenSensor::readValue() {
    uint16_t rawValue = analogRead(_pin);
    uint32_t voltage = uint32_t(VREF) * rawValue / ADC_RES;
    float temperature = _tempSensor->readValue();

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

    return (voltage * DO_Table[(int)temperature] / V_saturation);
}

void OxygenSensor::startCalibration(int points) {
    if (points > 0 && points <= MAX_CALIBRATION_POINTS) {
        calibrationPoints = points;
        currentCalibrationPoint = 0;
        calibrationState = CalibrationState::IN_PROGRESS;
        Logger::log(LogLevel::INFO, "O2 calibration started for " + String(points) + " point(s)");
    } else {
        Logger::log(LogLevel::ERROR, F("Invalid number of calibration points"));
    }
}

void OxygenSensor::saveCalibrationPoint() {
    if (calibrationState == CalibrationState::IN_PROGRESS) {
        if (currentCalibrationPoint < calibrationPoints) {
            uint8_t temp = _tempSensor->readValue();
            uint16_t rawValue = analogRead(_pin);
            uint16_t voltage = rawValue * (VREF / ADC_RES);
            
            calibrationVoltages[currentCalibrationPoint] = voltage;
            calibrationTemperatures[currentCalibrationPoint] = temp;
            
            currentCalibrationPoint++;
            
            Logger::log(LogLevel::INFO, "O2 calibration point " + String(currentCalibrationPoint) + 
                        F(" saved: V=") + String(voltage) + F("mV, T=") + String(temp) + F("°C"));
            
            if (currentCalibrationPoint == calibrationPoints) {
                Logger::log(LogLevel::INFO, F("All O2 calibration points saved. Use 'o2 calibrate finish' to complete."));
            }
        } else {
            Logger::log(LogLevel::WARNING, F("All O2 calibration points already saved. Use 'o2 calibrate finish' to complete."));
        }
    } else {
        Logger::log(LogLevel::ERROR, F("O2 calibration not started. Use 'o2 calibrate start <points>' first."));
    }
}

void OxygenSensor::finishCalibration() {
    if (calibrationState == CalibrationState::IN_PROGRESS && currentCalibrationPoint == calibrationPoints) {
        calibrationState = CalibrationState::COMPLETED;
        saveCalibrationToEEPROM();
        Logger::log(LogLevel::INFO, F("O2 calibration completed and saved."));
    } else {
        Logger::log(LogLevel::ERROR, F("Cannot finish O2 calibration. Not all points are saved."));
    }
}

String OxygenSensor::getCalibrationStatus() const {
    String status = F("O2 Calibration Status: ");
    switch (calibrationState) {
        case CalibrationState::NONE:
            status += F("Not calibrated");
            break;
        case CalibrationState::IN_PROGRESS:
            status += String(currentCalibrationPoint) + F(" of ") + String(calibrationPoints) + F(" points saved");
            break;
        case CalibrationState::COMPLETED:
            status += "Completed with " + String(calibrationPoints) + " point(s)";
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
    Logger::log(LogLevel::INFO, F("O2 calibration reset"));
}

void OxygenSensor::saveCalibrationToEEPROM() {
    int addr = 0;
    EEPROM.put(addr, calibrationPoints); addr += sizeof(int);
    for (int i = 0; i < calibrationPoints; i++) {
        EEPROM.put(addr, calibrationVoltages[i]); addr += sizeof(uint16_t);
        EEPROM.put(addr, calibrationTemperatures[i]); addr += sizeof(uint8_t);
    }
}

void OxygenSensor::loadCalibrationFromEEPROM() {
    int addr = 0;
    EEPROM.get(addr, calibrationPoints); addr += sizeof(int);
    if (calibrationPoints > 0 && calibrationPoints <= MAX_CALIBRATION_POINTS) {
        for (int i = 0; i < calibrationPoints; i++) {
            EEPROM.get(addr, calibrationVoltages[i]); addr += sizeof(uint16_t);
            EEPROM.get(addr, calibrationTemperatures[i]); addr += sizeof(uint8_t);
        }
        calibrationState = CalibrationState::COMPLETED;
        Logger::log(LogLevel::INFO, F("O2 calibration loaded from EEPROM"));
    } else {
        resetCalibration();
    }
}