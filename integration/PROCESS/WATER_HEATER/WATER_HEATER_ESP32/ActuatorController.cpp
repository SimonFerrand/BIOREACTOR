// ActuatorController.cpp
#include "ActuatorController.h"
#include <Arduino.h>
#include "Logger.h"

// Static pointers, initialized to nullptr
HeatingPlate* ActuatorController::heatingPlate = nullptr;


// Initialize method
bool ActuatorController::initialize(HeatingPlate& heatingP) {
    heatingPlate = &heatingP;
    if (heatingPlate->isOn()) {
        Logger::log(Logger::LogLevel::WARNING, F("Heating plate was ON during initialization - forcing OFF state"));
        heatingPlate->control(false, 0);
        delay(100);
    }
    
    Logger::log(Logger::LogLevel::INFO, F("Actuators initialized successfully"));
    return true;
}

bool ActuatorController::beginAll() {
    if (!heatingPlate) {
        Logger::log(Logger::LogLevel::ERROR, F("Actuators not initialized"));
        return false;
    }

    bool heatingPlateOk = false;
    
    for(int i = 0; i < 3; i++) {
        heatingPlate->begin();
        
        if (!heatingPlate->isOn() && heatingPlate->getCurrentValue() == 0) {
            heatingPlateOk = true;
            break;
        }
        
        Logger::log(Logger::LogLevel::WARNING, 
            "Heating plate initialization attempt " + String(i+1) + " failed");
        
        heatingPlate->control(false, 0);
        delay(1000);
    }

    if (!heatingPlateOk) {
        Logger::log(Logger::LogLevel::ERROR, F("Heating plate failed to initialize properly"));
        Logger::log(Logger::LogLevel::WARNING, F("System may run with limited functionality"));
    }

    Logger::log(Logger::LogLevel::INFO, 
        "Actuators started. Heating plate OK: " + String(heatingPlateOk));
    
    return true;
}

void ActuatorController::runActuator(const String& actuatorName, float value, int duration) {
    ActuatorInterface* actuator = findActuatorByName(actuatorName);
    if (actuator) {
        actuator->control(true, value);
        //Logger::log(Logger::LogLevel::INFO, "Running actuator: " + actuatorName + " with value: " + String(value));
        if (duration > 0) {
            delay(duration);
            stopActuator(actuatorName);
        }
    } else {
        Logger::log(Logger::LogLevel::ERROR, "Actuator not found: " + actuatorName);
    }
}

void ActuatorController::stopActuator(const String& actuatorName) {
    ActuatorInterface* actuator = findActuatorByName(actuatorName);
    if (actuator) {
        actuator->control(false, 0);
        delay(50);  // Ajoutez un court délai pour la stabilisation
        //Logger::log(Logger::LogLevel::INFO, "Stopped actuator: " + actuatorName);
    }
}

void ActuatorController::stopAllActuators() {
    const ActuatorInterface* actuators[] = {
        heatingPlate
    };
    for (const auto& actuator : actuators) {
        if (actuator->isOn()) {
            const_cast<ActuatorInterface*>(actuator)->control(false, 0);
            delay(50); 
        }
    }
    Logger::log(Logger::LogLevel::INFO, F("All actuators stopped"));
}

bool ActuatorController::isActuatorRunning(const String& actuatorName) {
    ActuatorInterface* actuator = findActuatorByName(actuatorName);
    return actuator ? actuator->isOn() : false;
}

int ActuatorController::getCurrentValue(const String& actuatorName) {
    ActuatorInterface* actuator = findActuatorByName(actuatorName);
    return actuator ? actuator->getCurrentValue() : 0;
}

ActuatorInterface* ActuatorController::findActuatorByName(const String& name) {
    static const ActuatorInterface* actuators[] = {
        heatingPlate
    };
    
    for (const auto& actuator : actuators) {
        if (name == actuator->getName()) return const_cast<ActuatorInterface*>(actuator);
    }
    return nullptr;
}

