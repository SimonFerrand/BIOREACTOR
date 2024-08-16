// ActuatorController.cpp
#include "ActuatorController.h"
#include "Logger.h"

// Static pointers, initialized to nullptr
DCPump* ActuatorController::airPump = nullptr;
DCPump* ActuatorController::drainPump = nullptr;
PeristalticPump* ActuatorController::nutrientPump = nullptr;
PeristalticPump* ActuatorController::basePump = nullptr;
StirringMotor* ActuatorController::stirringMotor = nullptr;
HeatingPlate* ActuatorController::heatingPlate = nullptr;
LEDGrowLight* ActuatorController::ledGrowLight = nullptr;
DCPump* ActuatorController::samplePump = nullptr;

// Initialize method
void ActuatorController::initialize(DCPump& airP, DCPump& drainP,
                                    PeristalticPump& nutrientP, PeristalticPump& baseP,
                                    StirringMotor& stirringM, HeatingPlate& heatingP,
                                    LEDGrowLight& ledLight, DCPump& sampleP) {
    // Assign addresses of actuators objects to the static pointers
    airPump = &airP;
    drainPump = &drainP;
    nutrientPump = &nutrientP;
    basePump = &baseP;
    stirringMotor = &stirringM;
    heatingPlate = &heatingP;
    ledGrowLight = &ledLight;
    samplePump = &sampleP;
}

void ActuatorController::beginAll() {
    airPump->begin();
    drainPump->begin();
    samplePump->begin();
    nutrientPump->begin();
    basePump->begin();
    stirringMotor->begin();
    heatingPlate->begin();
    ledGrowLight->begin();
}

void ActuatorController::runActuator(const String& actuatorName, float value, int duration) {
    //Logger::log(LogLevel::INFO, "Attempting to find actuator: " + actuatorName); ///
    ActuatorInterface* actuator = findActuatorByName(actuatorName);
    if (actuator) {
        //Logger::log(LogLevel::INFO, "Attempting to run actuator: " + actuatorName + " with value: " + String(value));
        actuator->control(true, value);
        Logger::log(LogLevel::INFO, "Running actuator: " + actuatorName + " with value: " + String(value));
        if (duration > 0) {
            delay(duration);
            stopActuator(actuatorName);
        }
    } else {
        Logger::log(LogLevel::ERROR, "Actuator not found: " + actuatorName);
    }
}

void ActuatorController::stopActuator(const String& actuatorName) {
    ActuatorInterface* actuator = findActuatorByName(actuatorName);
    if (actuator) {
        actuator->control(false, 0);
        delay(50);  // Ajoutez un court délai pour la stabilisation
        Logger::log(LogLevel::INFO, "Stopped actuator: " + actuatorName);
    }
}

void ActuatorController::stopAllActuators() {
    Logger::log(LogLevel::INFO, "Entering stopAllActuators");
    const ActuatorInterface* actuators[] = {
        airPump, drainPump, nutrientPump, basePump, 
        stirringMotor, heatingPlate, ledGrowLight
    };
    for (const auto& actuator : actuators) {
        if (actuator->isOn()) {
            Logger::log(LogLevel::INFO, "Stopping " + String(actuator->getName()));
            //actuator->control(false, 0);
            const_cast<ActuatorInterface*>(actuator)->control(false, 0);
            delay(50); 
        }
    }
    Logger::log(LogLevel::INFO, "All actuators stopped");
}

bool ActuatorController::isActuatorRunning(const String& actuatorName) {
    ActuatorInterface* actuator = findActuatorByName(actuatorName);
    return actuator ? actuator->isOn() : false;
}

ActuatorInterface* ActuatorController::findActuatorByName(const String& name) {
    static const ActuatorInterface* actuators[] = {
        airPump, drainPump, nutrientPump, basePump, 
        stirringMotor, heatingPlate, ledGrowLight, samplePump
    };
    
    for (const auto& actuator : actuators) {
        if (name == actuator->getName()) return const_cast<ActuatorInterface*>(actuator);
    }
    return nullptr;
}

float ActuatorController::getVolumeAdded(const String& actuatorName) {
    if (actuatorName == "nutrientPump" && nutrientPump) {
        return nutrientPump->getVolumeAdded();
    } else if (actuatorName == "basePump" && basePump) {
        return basePump->getVolumeAdded();
    }
    return 0.0f;
}

float ActuatorController::getVolumeRemoved(const String& actuatorName) {
    if (actuatorName == "drainPump" && drainPump) {
        return drainPump->getVolumeRemoved();
    }
    return 0.0f;
}

void ActuatorController::resetVolumeAdded(const String& actuatorName) {
    if (actuatorName == "nutrientPump" && nutrientPump) {
        nutrientPump->resetVolumeAdded();
    } else if (actuatorName == "basePump" && basePump) {
        basePump->resetVolumeAdded();
    }
}

void ActuatorController::resetVolumeRemoved(const String& actuatorName) {
    if (actuatorName == "drainPump" && drainPump) {
        drainPump->resetVolumeRemoved();
    }
}

float ActuatorController::getPumpMaxFlowRate(const String& actuatorName) {
    if (actuatorName == "nutrientPump" && nutrientPump) {
        return nutrientPump->getMaxFlowRate();
    } else if (actuatorName == "basePump" && basePump) {
        return basePump->getMaxFlowRate();
    }
    return 0.0f;
}

float ActuatorController::getPumpMinFlowRate(const String& actuatorName) {
    if (actuatorName == "nutrientPump" && nutrientPump) {
        return nutrientPump->getMinFlowRate();
    } else if (actuatorName == "basePump" && basePump) {
        return basePump->getMinFlowRate();
    }
    return 0.0f;
}

int ActuatorController::getStirringMotorMinRPM() {
    return stirringMotor ? stirringMotor->getMinRPM() : 0;
}

int ActuatorController::getStirringMotorMaxRPM() {
    return stirringMotor ? stirringMotor->getMaxRPM() : 0;
}

float ActuatorController::getTotalVolumeAdded() {
    return getVolumeAdded("nutrientPump") + getVolumeAdded("basePump");
}

float ActuatorController::getTotalVolumeRemoved() {
    return getVolumeRemoved("drainPump");
}


