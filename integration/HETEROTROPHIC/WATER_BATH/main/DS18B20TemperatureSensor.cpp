/*
 * DS18B20TemperatureSensor.cpp
 * This file provides the implementation of the DS18B20TemperatureSensor class defined in DS18B20TemperatureSensor.h.
 * The class reads temperature from the DS18B20 sensor.
 */

#include "DS18B20TemperatureSensor.h"

// Constructor for DS18B20TemperatureSensor
DS18B20TemperatureSensor::DS18B20TemperatureSensor(int pin, const char* name) : _ds(pin), _pin(pin), _name(name) {}

// Method to initialize the temperature sensor
void DS18B20TemperatureSensor::begin() {
    // Nothing specific to initialize for DS18B20 in this implementation
    Logger::log(Logger::LogLevel::INFO, String(_name) + F(" initialized"));
}

// Method to read the temperature from the sensor
float DS18B20TemperatureSensor::readValue() {
    byte data[12];
    byte addr[8];

    if (!_ds.search(addr)) {
        _ds.reset_search();
        //Serial.println("no DS18B20 temperature sensor found!");
        return -1000; // Return an error value if no sensor found
    }

    if (OneWire::crc8(addr, 7) != addr[7]) {
        //Serial.println("CRC is not valid (DS18B20 temperature sensor)!");
        return -2000; // Return an error value if CRC check fails
    }

    if (addr[0] != 0x10 && addr[0] != 0x28) {
        //Serial.print("Device (DS18B20 temperature sensor) is not recognized");
        return -3000; // Return an error value if the device is not recognized
    }

    _ds.reset();
    _ds.select(addr);
    _ds.write(0x44, 1); // Start temperature conversion

    delay(1000); // Wait for conversion to complete

    _ds.reset();
    _ds.select(addr);
    _ds.write(0xBE); // Read Scratchpad

    for (int i = 0; i < 9; i++) {
        data[i] = _ds.read();
    }

    _ds.reset_search();

    int16_t rawTemperature = (data[1] << 8) | data[0];
    float  temperature = rawTemperature / 16.0; // Convert raw temperature to Celsius
    return round(temperature * 10.0) / 10.0; // Round to 1 decimal place
}
