/*
 * ActuatorInterface.h
 * This file defines an abstract interface for actuators, providing a standard way
 * to control various actuators in the project. This interface can be implemented
 * by any actuator class, such as a pump, motor, or any other device that needs
 * control functionality.
 */

#ifndef ACTUATORINTERFACE_H
#define ACTUATORINTERFACE_H

class ActuatorInterface {
public:
    /*
     * Pure virtual function to initialize the actuator.
     * This should be called before using the actuator.
     */
    virtual void begin() = 0;

    /*
     * Pure virtual function to control the actuator.
     * @param state: Boolean indicating whether the actuator should be on or off.
     * @param value: Integer value to control the speed or intensity of the actuator.
     */
    virtual void control(bool state, int value) = 0;
    
    /*
     * Virtual function to check if the actuator is on.
     * @return: Boolean indicating the state of the actuator.
     */
    virtual bool isOn() const = 0;

    /*
     * Virtual function to get the name of the actuator.
     * @return: Constant character pointer to the name of the actuator.
     */
    virtual const char* getName() const = 0;

    /*
     * Virtual destructor to ensure proper cleanup of derived classes.
     */
    virtual ~ActuatorInterface() {}

    virtual int getCurrentValue() const = 0;
    
protected:
    bool _currentState;
    int _currentValue;
};

#endif