/*
 * HeatingPlate.h
 * This file defines a class for controlling a heating plate.
 * The class supports both on/off control and PWM control for future hardware.
 */

#ifndef HEATINGPLATE_H
#define HEATINGPLATE_H

#include "ActuatorInterface.h"
#include <Arduino.h>



class HeatingPlate : public ActuatorInterface {
public:
    /*
     * Constructor for HeatingPlate.
     * @param controlPin: The pin connected to the relay or PWM pin for the heating plate.
     * 
     * @param name: Identifier for the heating plate (for debugging purposes).
     */
     HeatingPlate(int relayPin, bool isPWMCapable, const char* name);

    /*
     * Method to initialize the heating plate.
     * This method is called to set up the heating plate before it's first used.
     */
    void begin() override;

    /*
     * Method to control the heating plate.
     * @param state: Boolean indicating whether the heating plate should be on or off.
     * @param value: Power percentage (0-100) if hardware is PWM capable, ignored otherwise.
     */
    void control(bool state, int value = 0) override;

    /*
     * Method to check if the heating plate is on.
     * @return Boolean indicating if the heating plate is on.
     */
    bool isOn() const override;

    /*
     * Method to get the name of the heating plate.
     * @return The name of the heating plate.
     */
    const char* getName() const override { return _name; }

private:
/*
    int _controlPin;   // Relay or PWM pin
    const char* _name;
    bool status;     // Track the state of the heating plate
    

    const int MAX_POWER_PERCENT = 100;
    // For cycle control (non-PWM hardware)
    const unsigned long DEFAULT_CYCLE_TIME = 10000; // 10 seconds per cycle = In process chemistry, it offers a good compromise between control accuracy and relay life. It is not necessary to increase it in most cases.
    unsigned long _cycleTime; 
    unsigned long lastCycleStart;
    double dutyCycle;

    void controlPWM(int value);
    void controlRelay(bool state);
    void controlWithCycle(double percentage);
    //int _lastPercentPower = 0;
    HeatingControlMode _mode;
    */
    int _relayPin;
    const char* _name;
    bool _status;
    bool _isPWMCapable;
    int _currentValue;
    
    unsigned long _cycleTime = 10000;
    unsigned long _lastCycleStart = 0;
    double _dutyCycle = 0;
    
    void controlPWM(int value);
    void controlOnOff(bool state);
    void controlWithCycle(int percentage);
};

#endif