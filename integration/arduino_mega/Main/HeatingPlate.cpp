/*
 * HeatingPlate.cpp
 * This file provides the implementation of the HeatingPlate class defined in HeatingPlate.h.
 * The class controls a heating plate using a relay.
 */

#include "HeatingPlate.h"

// Constructor for HeatingPlate
HeatingPlate::HeatingPlate(int relayPin, const char* id) : _relayPin(relayPin), _id(id) {
    pinMode(_relayPin, OUTPUT); // Set relay pin as output
    digitalWrite(_relayPin, LOW); // Ensure relay is off initially
}

// Method to control the heating plate
void HeatingPlate::control(bool state, int value) {
    if (state) {
        digitalWrite(_relayPin, HIGH); // Turn on the relay
        Serial.print(_id);
        Serial.println(" is ON");
    } else {
        digitalWrite(_relayPin, LOW); // Turn off the relay
        Serial.print(_id);
        Serial.println(" is OFF");
    }
}