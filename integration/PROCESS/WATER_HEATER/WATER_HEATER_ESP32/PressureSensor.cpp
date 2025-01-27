/**
 * PressureSensor.cpp
 * Implementation of the industrial pressure sensor interface.
 */

#include "PressureSensor.h"

PressureSensor::PressureSensor(uint8_t pin, const char* name)
    : _pin(pin)
    , _name(name) {
}

void PressureSensor::begin() {
    pinMode(_pin, INPUT);
    _lastReadTime = millis();
    
    // Initialize readings
    float initialVoltage = getRawVoltage();
    float initialPressure = voltageToPresure(initialVoltage);
    
    // Initialize signal processing
    _lastValidVoltage = initialVoltage;
    _lastValidPressure = initialPressure;
    _emaValue = initialPressure;
    
    // Initialize statistics
    resetStatistics();
    
    if(DEBUG_MODE) {
        Serial.println("Pressure sensor initialized:");
        Serial.print("Initial voltage: "); Serial.print(initialVoltage, 3); Serial.println("V");
        Serial.print("Initial pressure: "); Serial.print(initialPressure, 2); Serial.println(" bar");
    }
}

float PressureSensor::readValue() {
    // Get raw voltage and convert to pressure
    float voltage = getRawVoltage();
    float pressure = voltageToPresure(voltage);
    
    // Validate reading
    if (validateReading(pressure, voltage)) {
        // Apply EMA filter
        pressure = applyEMA(pressure);
        
        // Update statistics
        updateStatistics(pressure);
        
        // Store valid readings
        _lastValidPressure = pressure;
        _lastValidVoltage = voltage;
        _lastReadTime = millis();
    } else {
        // Use last valid reading if current reading is invalid
        pressure = _lastValidPressure;
    }
    
    return pressure;
}

float PressureSensor::getRawVoltage() const {
    return (readADC() * VOLTAGE_REFERENCE) / ADC_RESOLUTION;
}

uint16_t PressureSensor::readADC() const {
    uint16_t samples[ADC_SAMPLES];
    
    // Collect samples with delay between readings
    for(int i = 0; i < ADC_SAMPLES; i++) {
        samples[i] = analogRead(_pin);
        delayMicroseconds(200);
    }
    
    // Sort samples for median calculation
    for(int i = 0; i < ADC_SAMPLES-1; i++) {
        for(int j = i+1; j < ADC_SAMPLES; j++) {
            if(samples[i] > samples[j]) {
                uint16_t temp = samples[i];
                samples[i] = samples[j];
                samples[j] = temp;
            }
        }
    }
    
    // Calculate median by averaging central values
    uint32_t sum = 0;
    for(int i = ADC_DISCARD; i < ADC_SAMPLES - ADC_DISCARD; i++) {
        sum += samples[i];
    }
    
    return sum / (ADC_SAMPLES - (2 * ADC_DISCARD));
}

float PressureSensor::voltageToPresure(float voltage) const {
    // Apply linear offset correction
    float adjustedVoltage = voltage - OFFSET_V + NOMINAL_LOW_V;
    
    // Apply range limits
    if (adjustedVoltage <= NOMINAL_LOW_V) return 0.0f;
    if (adjustedVoltage >= NOMINAL_HIGH_V) return MAX_PRESSURE;
    
    // Linear conversion to pressure
    float pressure = (MAX_PRESSURE) * 
                    (adjustedVoltage - NOMINAL_LOW_V) / 
                    (NOMINAL_HIGH_V - NOMINAL_LOW_V);
    
    // Apply zero threshold
    if (abs(pressure) < ZERO_THRESHOLD) {
        pressure = 0.0f;
    }
    
    return pressure;
}

float PressureSensor::applyEMA(float newValue) {
    if (_stats.sampleCount == 0) {
        _emaValue = newValue;
    } else {
        _emaValue = (EMA_ALPHA * newValue) + ((1.0f - EMA_ALPHA) * _emaValue);
    }
    return _emaValue;
}

void PressureSensor::updateStatistics(float pressure) {
    _stats.sampleCount++;
    _stats.lastReadingTime = millis();
    
    // Update min/max
    if (pressure < _stats.minPressure) _stats.minPressure = pressure;
    if (pressure > _stats.maxPressure) _stats.maxPressure = pressure;
    
    // Update running average and variance
    float delta = pressure - _stats.avgPressure;
    _stats.avgPressure += delta / _stats.sampleCount;
    float delta2 = pressure - _stats.avgPressure;
    _stats.variance += (delta * delta2 - _stats.variance) / _stats.sampleCount;
}

bool PressureSensor::validateReading(float pressure, float voltage) {
    _errorFlags = 0;
    bool isValid = true;

    // Check voltage limits
    if (voltage < FAULT_LOW_V) {
        if(DEBUG_MODE) {
            Serial.print("Debug - Voltage low: ");
            Serial.print(voltage, 3);
            Serial.print("V < ");
            Serial.print(FAULT_LOW_V, 3);
            Serial.println("V");
        }
        _errorFlags |= static_cast<uint8_t>(PressureError::DISCONNECTED);
        isValid = false;
    }
    if (voltage > FAULT_HIGH_V) {
        _errorFlags |= static_cast<uint8_t>(PressureError::OVER_RANGE);
        isValid = false;
    }
    
    // Check pressure range
    if (pressure < MIN_PRESSURE || pressure > MAX_PRESSURE) {
        _errorFlags |= static_cast<uint8_t>(PressureError::OUT_OF_BOUNDS);
        isValid = false;
    }
    
    // Check rate of change
    if (_lastReadTime > 0) {
        float timeDelta = (millis() - _lastReadTime) / 1000.0f;
        float rateOfChange = abs(pressure - _lastValidPressure) / timeDelta;
        
        if (rateOfChange > _maxRateOfChange) {
            _errorFlags |= static_cast<uint8_t>(PressureError::RAPID_CHANGE);
            isValid = false;
        }
    }
    
    return isValid;
}

void PressureSensor::resetStatistics() {
    _stats = {
        MAX_PRESSURE,    // minPressure
        MIN_PRESSURE,    // maxPressure
        0.0f,           // avgPressure
        0.0f,           // variance
        0,              // sampleCount
        0               // lastReadingTime
    };
    _emaValue = 0.0f;
}