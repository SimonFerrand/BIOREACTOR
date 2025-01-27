/**
 * Industrial Pressure Sensor Monitoring System
 * For ESP32-S2-WROOM-1 with 4-20mA pressure sensor (0-5 bar range)
 * using DFRobot SEN0262 current-to-voltage converter
 * 
 * POWER SUPPLY CONNECTIONS:
 * 1. ESP32-S2 Power (CHOOSE ONE, NEVER BOTH):
 *    Option A (Recommended):
 *    - Power via USB
 *    - 3V3 pins become outputs (3.3V)
 *    
 *    Option B:
 *    - Power via external 3.3V to 3V3 pin
 *    - CAUTION: Must be regulated 3.3V
 *    - DO NOT use USB power simultaneously
 * 
 * HARDWARE CONNECTIONS:
 * 1. Pressure Sensor (2-wire 4-20mA):
 *    - PIN1 (Red) → +24V DC power supply
 *    - PIN2 (Black) → SEN0262 IN+
 * 
 * 2. SEN0262 Connections:
 *    - VCC → ESP32-S2 3V3 pin (output when USB-powered)
 *    - GND → ESP32-S2 GND
 *    - OUT → ESP32-S2 GPIO8 (ADC1_CH7)
 *    - IN- → Power supply GND (24V-)
 *    - IN+ → Pressure sensor PIN2
 * 
 * 3. Power Supply (24V):
 *    - Positive → Pressure sensor PIN1
 *    - Negative → SEN0262 IN-
 * 
 * SAFETY NOTES:
 * - Always connect GND (24V power supply, ESP32, and SEN0262)
 * - Verify 24V power supply polarity
 * - Don't exceed SEN0262 input range (25mA max)
 * - Don't power ESP32 from USB and 3V3 simultaneously
 * 
 * Libraries Required:
 * - CircularBuffer.hpp (for moving average filter)
 * 
 * 
 * How it works:
 * 1. The pressure sensor converts pressure (0-5 bar) to current (4-20mA)
 * 2. SEN0262 converts the current to voltage (0.48V-2.4V)
 * 3. ESP32 reads voltage through ADC
 * 4. Program applies:
 *    - Moving average filter for noise reduction
 *    - Error detection (disconnection, over-range, rapid changes)
 *    - Safety monitoring and alerts
 * 
 * Safety Features:
 * - Detects sensor disconnection (current < 3.8mA)
 * - Detects over-current conditions (current > 20.5mA)
 * - Monitors pressure change rate
 * - Validates readings against physical limits
 * - Moving average filtering for noise reduction
 */

#include <Arduino.h>
#include "PressureSensor.h"
#include "SafetySystem.h"
#include "SensorController.h"

// Pin Definitions
const uint8_t PRESSURE_SENSOR_PIN = 35;  // ADC1_CH7

// Global Objects
PressureSensor pressureSensor(PRESSURE_SENSOR_PIN);
SafetySystem safetySystem;

// Setup function
void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    while(!Serial) {
        ; // Wait for serial port to connect
    }
    
    // Initialize pressure sensor
    pressureSensor.begin();
    
    // Initialize sensor controller
    SensorController::initialize(pressureSensor);
    SensorController::beginAll();
    
    // Print initialization message
    Serial.println("\nPressure monitoring system initialized");
    Serial.println("Reading from pressure sensor (0-5 bar)");
    Serial.println("----------------------------------------");
}

void loop() {
    // Read raw ADC and voltage for debugging
    uint16_t rawADC = analogRead(PRESSURE_SENSOR_PIN);
    float voltage = (rawADC * 3.3) / 4095.0;
    
    // Read pressure through sensor controller
    float pressure = SensorController::readSensor("pressureSensor");
    
    // Print detailed debug information
    Serial.println("\n--- Sensor Reading ---");
    Serial.print("Raw ADC: ");
    Serial.print(rawADC);
    Serial.print(" (0-4095)\n");
    
    Serial.print("Voltage: ");
    Serial.print(voltage, 3);
    Serial.print("V (Expected: 0.48V-2.4V)\n");
    
    Serial.print("Pressure: ");
    Serial.print(pressure, 2);
    Serial.println(" bar (Range: 0-5 bar)\n");

    PressureStats stats = pressureSensor.getStatistics();
    Serial.print("Min: "); Serial.print(stats.minPressure, 2);
    Serial.print(" bar, Max: "); Serial.print(stats.maxPressure, 2);
    Serial.print(" bar, Avg: "); Serial.print(stats.avgPressure, 2);
    Serial.println(" bar");
    
    // Check safety limits
    safetySystem.checkLimits();
    
    delay(1000);
}