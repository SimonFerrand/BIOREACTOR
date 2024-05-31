/*
 * HeatingPlate.h
 * This file defines a class for controlling a heating plate.
 * The class implements the ActuatorInterface and provides methods to turn the heating plate on or off.
 *
 * Physical modules used:
 * - Relay module connected to the heating plate
 */

#ifndef HEATINGPLATE_H
#define HEATINGPLATE_H

#include "ActuatorInterface.h"
#include <Arduino.h>

class HeatingPlate : public ActuatorInterface {
public:
    /*
     * Constructor for HeatingPlate.
     * @param relayPin: The pin connected to the relay controlling the heating plate.
     * @param id: Identifier for the heating plate (for debugging purposes).
     */
    HeatingPlate(int relayPin, const char* id);

    /*
     * Method to control the heating plate.
     * @param state: Boolean indicating whether the heating plate should be on or off.
     * @param value: Not used in this implementation.
     */
    void control(bool state, int value = 0);

private:
    int _relayPin;   // Relay pin
    const char* _id; // Identifier for the heating plate
};

#endif