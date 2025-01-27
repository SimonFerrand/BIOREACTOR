// ActuatorController.h
#ifndef ACTUATOR_CONTROLLER_H
#define ACTUATOR_CONTROLLER_H

#include <Arduino.h>
#include "HeatingPlate.h"


enum class ControlMode {
    PWM,
    Relay
};

class ActuatorController {
public:
    public:
    static bool initialize(HeatingPlate& heatingPlate);
    static bool beginAll();
    
    static void runActuator(const String& actuatorName, float value, int duration);
    static void stopActuator(const String& actuatorName);
    static void stopAllActuators();
    static bool isActuatorRunning(const String& actuatorName);
    static int getCurrentValue(const String& actuatorName);

    //template<typename T> // SUPPRIMER??

    static ActuatorInterface* findActuatorByName(const String& name);
    static void runHeatingPlatePID(double pidOutput);

private:
    static HeatingPlate* heatingPlate;
};

#endif // ACTUATOR_CONTROLLER_H

