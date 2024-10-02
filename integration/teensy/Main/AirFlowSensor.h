/*
 * AirFlowSensor.h
 * This file defines a class for reading the air flow rate from a YF-S401 air flow meter.
 * The class implements the SensorInterface and provides methods to initialize the sensor and read the flow rate.
 *
 * Physical modules used:
 * - YF-S401 Air Flow Meter
 */

/*
  Installation Instructions:

  Instructions to connect the 3-wire air flow meter (YF-S401) to the Arduino:
  https://www.epitran.it/ebayDrive/datasheet/YF-S401.pdf

  1. Connections for the 3-wire air flow meter to Arduino:
     - Red Wire (Power): Connect to 5V on the Arduino.
     - Black Wire (Ground): Connect to GND on the Arduino.
     - Yellow Wire (Signal): Connect to a digital input pin (e.g., pin 2) on the Arduino.
  
  2. The air flow meter provides a pulsed output proportional to the flow rate.
     This code will count the pulses over a specific time period to calculate the flow rate.

  Flow rate range: 1 to 5 liters per minute.
  Measurement error: ±2%.
  Pulses per liter: 5880 pulses/L. (=>Frequency: F = 98 * Q (L / Min)). F = (98 * Q) ± 2% avec Q = L / MIN
  Pressure conversion: 0.8 MPa = 8 bar
*/

#ifndef AIRFLOWSENSOR_H
#define AIRFLOWSENSOR_H

#include "SensorInterface.h"
#include <Arduino.h>

class AirFlowSensor : public SensorInterface {
public:
    /*
     * Constructor for AirFlowSensor.
     * @param pin: The digital pin connected to the air flow meter's signal wire.
     */
    AirFlowSensor(int pin, const char* name);

    /*
     * Method to initialize the air flow meter sensor.
     */
    void begin() override;

    /*
     * Method to read the flow rate from the sensor.
     * @return: The flow rate in liters per minute (L/min).
     */
    float readValue();

    /*
     * Interrupt service routine to count pulses.
     */
    static void countPulses();

    const char* getName() const override { return _name; }

private:
    int _pin; // Digital pin connected to the air flow meter's signal wire
    volatile unsigned long _pulseCount; // Pulse count from the air flow meter
    unsigned long _lastTime; // Last time the flow rate was calculated
    static const unsigned long _interval = 1000; // Interval to calculate flow rate (in milliseconds)
    static const float _pulsesPerLiter; // Pulses per liter as per the sensor's specification

    static AirFlowSensor* instance; // Static instance for the interrupt handler
    const char* _name;
};

#endif
