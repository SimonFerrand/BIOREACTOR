/*
 * LEDGrowLight.h
 * This file defines a class for controlling a LED grow light.
 * The class implements the ActuatorInterface and provides methods to turn the LED grow light on or off and control its intensity.
 *
 * Physical modules used:
 * - Relay module connected to the LED grow light
 *
 * The 2835 LED chip emits light in the red and blue spectrum, which are crucial for plant growth. 
 * Red light (around 660 nm) helps in flowering and fruit production, while blue light (around 450 nm) is essential for vegetative growth.
 */

#ifndef LEDGROWLIGHT_H
#define LEDGROWLIGHT_H

#include "ActuatorInterface.h"
#include <Arduino.h>

class LEDGrowLight : public ActuatorInterface {
public:
    /*
     * Constructor for LEDGrowLight.
     * @param relayPin: The pin connected to the relay controlling the LED grow light.
     * @param name: Identifier for the LED grow light (for debugging purposes).
     */
    LEDGrowLight(int relayPin, const char* name);

    /*
     * Method to initialize the LED grow light.
     * This method is called to set up the LED grow light before it's first used.
     */
    void begin() override;

    /*
     * Method to control the LED grow light.
     * @param state: Boolean indicating whether the LED grow light should be on or off.
     * @param value: Intensity percentage (0-100).
     */
    void control(bool state, int value = 0) override;

    /*
     * Method to check if the LED grow light is on.
     * @return Boolean indicating if the LED grow light is on.
     */
    bool isOn() const override;

    /*
     * Method to get the name of the LED grow light.
     * @return The name of the LED grow light.
     */
    const char* getName() const override { return _name; }

private:
    int _relayPin;   // Relay pin
    const char* _name;
    bool _status;     // Track the state of the LED grow light
    int _intensity ;   // Current intensity of the LED grow light (0-100)
};

#endif