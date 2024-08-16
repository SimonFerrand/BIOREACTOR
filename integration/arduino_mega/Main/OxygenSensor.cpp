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

// Constructor for OxygenSensor
OxygenSensor::OxygenSensor(int pin, PT100Sensor* tempSensor, const char* name) : _pin(pin), _tempSensor(tempSensor), _name(name) {}

// Method to initialize the DO sensor
void OxygenSensor::begin() {
    // Nothing specific to initialize for DO sensor in this implementation
    Logger::log(LogLevel::INFO, String(_name) + " initialized");
}

// Method to read the DO value from the sensor
float OxygenSensor::readValue() {
    uint16_t rawValue = analogRead(_pin); // Read the analog value from DO sensor
    uint32_t voltage = uint32_t(VREF) * rawValue / ADC_RES; // Convert ADC value to voltage
    float temperature = _tempSensor->readValue(); // Read temperature from the PT100 sensor

    uint32_t V_saturation;
    if (TWO_POINT_CALIBRATION == 0) {
        V_saturation = (uint32_t)CAL1_V + (uint32_t)35 * temperature - (uint32_t)CAL1_T * 35;
    } else {
        V_saturation = (int16_t)((int8_t)temperature - CAL2_T) * ((uint32_t)CAL1_V - CAL2_V) / ((uint8_t)CAL1_T - CAL2_T) + CAL2_V;
    }

    return (voltage * DO_Table[(int)temperature] / V_saturation); // Calculate DO concentration
}

// Method for calibration
void OxygenSensor::calibrate() {
    uint32_t raw = analogRead(_pin);
    Serial.println("raw:\t" + String(raw) + "\tVoltage(mv):\t" + String(raw * VREF / ADC_RES));
}
