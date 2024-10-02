/*
 * PeristalticPump.h
 * This file defines a class for controlling peristaltic pumps using a DAC (MCP4725) and a relay. 
 * The DAC is connected to the Pulse Generation Control Board, which regulates the peristaltic pump equipped with a stepper motor through a two-phase DC stepping driver.
 * The class implements the ActuatorInterface and provides methods to control the pump flow rate in ml/min.
 * Each pump uses a different DAC address to allow for independent control (only two addresses available for the factory version of this DAC).
 * The class provides methods to control the pump's state (on/off) and flow rate.
 *
 * Physical modules used:
 * - Adafruit MCP4725 DAC
 * - Relay module
 * - Peristaltic pump (Pulse Generation Control Board + two-phase DC stepping driver + stepper motor)
 */

#ifndef PERISTALTICPUMP_H
#define PERISTALTICPUMP_H

#include "ActuatorInterface.h"
#include <Adafruit_MCP4725.h>
#include <Arduino.h>

class PeristalticPump : public ActuatorInterface {
public:
    /*
     * Constructor for PeristalticPump.
     * @param dacAddress: I2C address of the MCP4725 DAC.
     * @param relayPin: The pin connected to the relay controlling the pump's power.
     * @param maxFlowRate: The maximum flow rate of the pump in ml/min.
     * @param id: Identifier for the pump (for debugging purposes).
     */
    PeristalticPump(uint8_t dacAddress, int relayPin, float _minFlowRate, float maxFlowRate, const char* name);

    /*
     * Initializes the peristaltic pump by setting up the relay pin and the DAC.
     */
    void begin();

    /*
     * Controls the pump's state and flow rate.
     * @param state: Boolean indicating whether the pump should be on or off.
     * @param flowRate: Desired flow rate in ml/min.
     */
    void control(bool state, int value) override;

    /*
     * Method to check if the pump is on.
     * @return Boolean indicating if the pump is on.
     */
    bool isOn() const override;

    const char* getName() const override { return _name; }

    float getVolumeAdded() const { return _volumeAdded; }  // in liters
    void resetVolumeAdded() { _volumeAdded = 0; }

    float getMaxFlowRate() const { return _maxFlowRate; }
    float getMinFlowRate() const { return _minFlowRate; }


private:
    uint8_t _dacAddress;    // I2C address of the DAC
    int _relayPin;          // Relay pin
    float _maxFlowRate;     // Maximum flow rate of the pump
    float _minFlowRate;     // Minimum flow rate of the pump
    const char* _name;
    Adafruit_MCP4725 _dac;  // DAC instance
    bool _status;            // Track the state of the pump
    float _currentFlowRate;
    float _volumeAdded;      // Track the volume added by the pump
    
    /*
     * Converts flow rate in ml/min to DAC value.
     * @param flowRate: Desired flow rate in ml/min.
     * @return: Corresponding DAC value.
     */
    uint16_t flowRateToDAC(float _flowRate);
};

#endif
