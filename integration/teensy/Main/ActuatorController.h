// ActuatorController.h
#ifndef ACTUATOR_CONTROLLER_H
#define ACTUATOR_CONTROLLER_H

#include <Arduino.h>
#include "DCPump.h"
#include "PeristalticPump.h"
#include "StirringMotor.h"
#include "HeatingPlate.h"
#include "LEDGrowLight.h"
#include "ActuatorController.h"

enum class ControlMode {
    PWM,
    Relay
};

class ActuatorController {
public:
    public:
    static void initialize(DCPump& airPump, DCPump& drainPump,
                           PeristalticPump& nutrientPump, PeristalticPump& basePump,
                           StirringMotor& stirringMotor, HeatingPlate& heatingPlate,
                           LEDGrowLight& ledGrowLight, DCPump& samplePump, DCPump& fillPump);
    static void beginAll();
    
    static void runActuator(const String& actuatorName, float value, int duration);
    static void stopActuator(const String& actuatorName);
    static void stopAllActuators();
    static bool isActuatorRunning(const String& actuatorName);
    static int getCurrentValue(const String& actuatorName);

    template<typename T>
    static float getPumpParameter(const String& actuatorName, float (T::*getter)() const);

    static float getVolumeAdded(const String& actuatorName);
    static float getVolumeRemoved(const String& actuatorName);
    static void resetVolumeAdded(const String& actuatorName);
    static void resetVolumeRemoved(const String& actuatorName);
    static float getTotalVolumeAdded();
    static float getTotalVolumeRemoved();

    static float getPumpMaxFlowRate(const String& actuatorName);
    static float getPumpMinFlowRate(const String& actuatorName);
    static int getStirringMotorMinRPM();
    static int getStirringMotorMaxRPM();

    static ActuatorInterface* findActuatorByName(const String& name);
    static void runHeatingPlatePID(double pidOutput);

private:
    static DCPump* airPump;
    static DCPump* drainPump;
    static PeristalticPump* nutrientPump;
    static PeristalticPump* basePump;
    static StirringMotor* stirringMotor;
    static HeatingPlate* heatingPlate;
    static LEDGrowLight* ledGrowLight;
    static ControlMode heatingControlMode;
    static DCPump* samplePump;
    static DCPump* fillPump;
};

#endif // ACTUATOR_CONTROLLER_H

