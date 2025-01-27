/**
 * PressureSensor.h
 * Industrial pressure sensor interface for 4-20mA sensors.
 * Designed for use with ESP32-S2 and DFRobot SEN0262 current-to-voltage converter.
 * 
 * Features:
 * - Handles 4-20mA pressure sensor with 0-5 bar range
 * - Median filtering for ADC readings
 * - EMA (Exponential Moving Average) for smooth output
 * - Comprehensive error detection
 * - Statistical analysis of readings
 * - Zero threshold detection
 */

#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

#include <Arduino.h>
#include <CircularBuffer.hpp>
#include "SensorInterface.h"

// Pressure sensor error flags
enum class PressureError {
    NONE = 0x00,
    DISCONNECTED = 0x01,    // Current < 3.8mA (0.456V)
    OVER_RANGE = 0x02,      // Current > 20.5mA (2.46V)
    RAPID_CHANGE = 0x04,    // Value changed too quickly
    OUT_OF_BOUNDS = 0x08,   // Value outside physical limits
    ADC_ERROR = 0x10        // ADC reading error
};

// Statistics structure
struct PressureStats {
    float minPressure;
    float maxPressure;
    float avgPressure;
    float variance;
    unsigned long sampleCount;
    unsigned long lastReadingTime;
};

class PressureSensor : public SensorInterface {
public:
    // Constructor
    PressureSensor(uint8_t pin, const char* name = "pressureSensor");

    // SensorInterface implementation
    void begin() override;
    float readValue() override;
    const char* getName() const override { return _name; }

    // Status methods
    uint8_t getErrorFlags() const { return _errorFlags; }
    bool isHealthy() const { return _errorFlags == 0; }
    float getRawVoltage() const;
    void setMaxRateOfChange(float maxRatePerSecond) { _maxRateOfChange = maxRatePerSecond; }
    
    // Statistics methods
    PressureStats getStatistics() const { return _stats; }
    void resetStatistics();

private:
    // Hardware configuration
    const uint8_t _pin;
    const char* _name;

    // ADC constants
    static constexpr float ADC_RESOLUTION = 4095.0f;    // 12-bit ADC
    static constexpr float VOLTAGE_REFERENCE = 3.3f;    // ESP32 reference voltage

    // Current thresholds (mA)
    static constexpr float FAULT_LOW_MA = 3.5f;        // Fault detection low current
    static constexpr float FAULT_HIGH_MA = 20.5f;      // Fault detection high current
    static constexpr float NOMINAL_LOW_MA = 4.0f;      // Normal range low current
    static constexpr float NOMINAL_HIGH_MA = 20.0f;    // Normal range high current

    // Voltage thresholds (V = I * 120Ω)
    static constexpr float FAULT_LOW_V = 0.330f;       // Adjusted for real readings
    static constexpr float FAULT_HIGH_V = 2.46f;       // 20.5mA * 120Ω
    static constexpr float NOMINAL_LOW_V = 0.430f;     // ADJUSTED FOR ZERO READING 
    static constexpr float NOMINAL_HIGH_V = 2.4f;      // 20mA * 120Ω
    static constexpr float OFFSET_V = NOMINAL_LOW_V;   // Use NOMINAL_LOW_V as offset reference          

    // Pressure range and thresholds
    static constexpr float MIN_PRESSURE = 0.0f;        // Minimum pressure in bar
    static constexpr float MAX_PRESSURE = 5.0f;        // Maximum pressure in bar
    static constexpr float ZERO_THRESHOLD = 0.02f;     // Zero detection threshold (bar)

    // Signal processing parameters
    static constexpr float EMA_ALPHA = 0.1f;           // EMA smoothing factor
    static constexpr uint8_t FILTER_SIZE = 20;         // Circular buffer size
    static constexpr int ADC_SAMPLES = 30;             // Number of ADC samples for median
    static constexpr int ADC_DISCARD = 5;              // Number of extreme values to discard

    // Error detection
    float _maxRateOfChange = 0.5f;                     // Maximum bar/second change
    uint32_t _lastReadTime = 0;                        // Last reading timestamp
    float _lastValidPressure = 0.0f;                   // Last valid pressure
    float _lastValidVoltage = 0.0f;                    // Last valid voltage
    uint8_t _errorFlags = 0;                           // Current error flags
    
    // Signal processing
    CircularBuffer<float, FILTER_SIZE> _samples;       // Circular buffer for EMA
    float _emaValue = 0.0f;                           // Current EMA value

    // Statistics
    PressureStats _stats = {
        MAX_PRESSURE,    // minPressure
        MIN_PRESSURE,    // maxPressure
        0.0f,           // avgPressure
        0.0f,           // variance
        0,              // sampleCount
        0              // lastReadingTime
    };

    // Debug configuration
    static constexpr bool DEBUG_MODE = true;

    // Private methods
    float voltageToPresure(float voltage) const;
    float applyEMA(float newValue);
    bool validateReading(float pressure, float voltage);
    uint16_t readADC() const;
    void updateStatistics(float pressure);
};

#endif // PRESSURE_SENSOR_H