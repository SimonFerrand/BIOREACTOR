#include "FermentationProgram.h"
#include <Arduino.h>
#include "Logger.h"


/*
# Dextrose feed parameters
Target (450 g/l)
 - Total volume to be injected: 146.67 ml
 - Desired duration: 24 hours
 - Average hourly volume: 146.67 ml ÷ 24 = 6.11 ml/h
Calculation of optimum parameters
 1- For regular addition over 24 hours :
    - Total cycle: 5 minutes 
    - Number of cycles over 24h: 288 cycles
    - Volume per cycle: 146.67 ml ÷ 288 = 0.51 ml/cycle

 2- With a reasonable flow rate of 3 ml/min :
    - Activation time required: (0.51 ml ÷ 3 ml/min) × 60 = 10.2 seconds
    - Rounded to 10 seconds for simplicity

 3- Setting
static const unsigned long NUTRIENT_ACTIVATION_TIME = 10000;  // 10 secondes
static const unsigned long NUTRIENT_PAUSE_TIME = 290000;      // 290 secondes (4 min 50 sec)
static constexpr float DEFAULT_NUTRIENT_FLOW_RATE = 3.0;     // 3 ml/min

 4- Solution
    - 60 g of glucose = 66g of dextrose monohydrate 
    - maximum solubility at 25°C : 470g/L
    -> 66g og monohydrate dextrose / 450 g/L = 146.67 ml or 90g in 200 mL

    

# NaOH 1.5% :
    - Place 150ml of distilled water in a clean 200ml bottle.
    - Add 10ml NaOH 30.5%, then make up to 200ml with water and mix gently.

*/

FermentationProgram::FermentationProgram(PIDManager& pidManager, VolumeManager& volumeManager)
    : ProgramBase(),
      pidManager(pidManager),
      volumeManager(volumeManager),
      nutrientFixedFlowRate(DEFAULT_NUTRIENT_FLOW_RATE),
      totalPauseTime(0),
      tempSetpoint(0),
      phSetpoint(0),
      doSetpoint(0),
      nutrientConc(0),
      baseConc(0),
      duration(0),
      startTime(0),
      pauseStartTime(0),
      isPIDEnabled(true), //PID activated by default ; false/true
      lastNutrientActivationTime(0),
      currentStirringSpeed(0)
{
}

void FermentationProgram::configure(float tempSetpoint, float phSetpoint, float doSetpoint,
                                    float nutrientConc, float baseConc, float durationHours, float nutrientDelayHours,
                                    const String& experimentName, const String& comment) {
    this->tempSetpoint = tempSetpoint;
    this->phSetpoint = phSetpoint;
    this->doSetpoint = doSetpoint;
    this->nutrientConc = nutrientConc;
    this->baseConc = baseConc;
    this->duration = static_cast<unsigned long>(durationHours * 3600000.0); // Convert hours to milliseconds
    this->nutrientStartDelay = static_cast<unsigned long>(nutrientDelayHours * 3600000.0); // Convert hours to milliseconds
    this->experimentName = experimentName;
    this->comment = comment;

    pidManager.setTemperatureSetpoint(tempSetpoint);
    pidManager.setPHSetpoint(phSetpoint);
    pidManager.setDOSetpoint(doSetpoint);

    Logger::log(LogLevel::INFO, "Fermentation configured:");
    Logger::log(LogLevel::INFO, "Temperature setpoint: " + String(tempSetpoint));
    Logger::log(LogLevel::INFO, "pH setpoint: " + String(phSetpoint));
    Logger::log(LogLevel::INFO, "DO setpoint: " + String(doSetpoint));
    Logger::log(LogLevel::INFO, "Nutrient concentration: " + String(nutrientConc));
    Logger::log(LogLevel::INFO, "Base concentration: " + String(baseConc));
    Logger::log(LogLevel::INFO, "Duration: " + String(duration) + " seconds");
    Logger::log(LogLevel::INFO, "Nutrient addition delay: " + String(nutrientStartDelay) + " milliseconds");
    Logger::log(LogLevel::INFO, "Experiment name: " + experimentName);
    Logger::log(LogLevel::INFO, "Comment: " + comment);

    Logger::log(LogLevel::INFO, "Configure - Duration set to: " + String(this->duration));
}

void FermentationProgram::start(const String& command) {
    parseCommand(command);
    Logger::log(LogLevel::INFO, "Start - After parseCommand, Duration value: " + String(duration) + " secondes");

    if (volumeManager.getCurrentVolume() == 0) {
      //Logger::log(LogLevel::ERROR, "Initial volume not set. Please set initial volume before starting fermentation.");
      Logger::log(LogLevel::ERROR, F("Initial volume not set. Please set initial volume before starting fermentation."));
      return;
    }
    _isRunning = true;
    _isPaused = false;
    startTime = millis();
    totalPauseTime = 0;
    nutrientAdditionStarted = false;

    initializeStirringSpeed();

    if (isPIDEnabled) {
        pidManager.startTemperaturePID(tempSetpoint);
        pidManager.startPHPID(phSetpoint);
        pidManager.startDOPID(doSetpoint);
        }
    
    //ActuatorController::runActuator("airPump", 50, 0);  // Start air pump at 50% speed
    //ActuatorController::runActuator("stirringMotor", 390, 0);  // Start stirring at 390 RPM

    /*
    Logger::log(LogLevel::INFO, "Fermentation started: " + experimentName);
    Logger::log(LogLevel::INFO, "Comment: " + comment);
    Logger::log(LogLevel::INFO, "Fermentation parameters:");
    Logger::log(LogLevel::INFO, "  Duration: " + String(duration) + " seconds");
    Logger::log(LogLevel::INFO, "  Initial volume: " + String(volumeManager.getCurrentVolume()) + " L");
    Logger::log(LogLevel::INFO, "  Max allowed volume: " + String(volumeManager.getMaxAllowedVolume()) + " L");
    Logger::log(LogLevel::INFO, "  Min volume: " + String(volumeManager.getMinVolume()) + " L");
    Logger::log(LogLevel::INFO, "Duration set to: " + String(duration) + " seconds");
    */


    Logger::log(LogLevel::INFO, F("Fermentation parameters:"));
    Logger::log(LogLevel::INFO, String(F("   - Duration: ")) + String(duration) + F(" seconds"));
    Logger::log(LogLevel::INFO, String(F("   - Nutrient addition delay: ")) + String(getNutrientStartDelay()) + F(" hours"));
    Logger::log(LogLevel::INFO, String(F("   - Initial volume: ")) + String(volumeManager.getCurrentVolume()) + F(" L"));
    Logger::log(LogLevel::INFO, String(F("   - Max allowed volume: ")) + String(volumeManager.getMaxAllowedVolume()) + F(" L"));
    Logger::log(LogLevel::INFO, String(F("   - Min volume: ")) + String(volumeManager.getMinVolume()) + F(" L"));
    Logger::log(LogLevel::INFO, String(F("   - Experiment Name: ")) + experimentName);
    Logger::log(LogLevel::INFO, String(F("   - Comment: ")) + comment);
}

void FermentationProgram::update() {
    if (!_isRunning || _isPaused) return;

    if (isPIDEnabled) {
        pidManager.updateAllPIDControllers();
        pidManager.adjustPIDStirringSpeed();
    }
    // We do nothing if isPIDEnabled is false

    updateVolume();
    checkCompletion();

    // Check if it's time to start adding nutrients
    unsigned long elapsedTime = millis() - startTime - totalPauseTime;
    if (!nutrientAdditionStarted && elapsedTime >= nutrientStartDelay) {
        nutrientAdditionStarted = true;
        Logger::log(LogLevel::INFO, "Starting nutrient addition after delay of " + 
                   String(getNutrientStartDelay()) + " hours");
    }
    // Add nutrients only when the deadline has passed
    if (nutrientAdditionStarted) {
        addNutrientsContinuouslyFixedRate(nutrientFixedFlowRate);
    }
    
}

void FermentationProgram::pause() {
    if (!_isRunning || _isPaused) return;

    _isPaused = true;
    pauseStartTime = millis();

    pidManager.pauseAllPID();

    ActuatorController::runActuator("stirringMotor", 390, 0);  // Minimum stirring speed
    ActuatorController::stopActuator("airPump");
    ActuatorController::stopActuator("nutrientPump");
    ActuatorController::stopActuator("basePump");

    //Logger::log(LogLevel::INFO, "Fermentation paused");
    Logger::log(LogLevel::INFO, F("Fermentation paused"));
}

void FermentationProgram::resume() {
    if (!_isRunning || !_isPaused) return;

    _isPaused = false;
    totalPauseTime += millis() - pauseStartTime;

    pidManager.resumeAllPID();

    ActuatorController::runActuator("airPump", 30, 0);  // Resume air pump at 30% speed

    //Logger::log(LogLevel::INFO, "Fermentation resumed");
    Logger::log(LogLevel::INFO, F("Fermentation resumed"));
}

void FermentationProgram::stop() {
    if (!_isRunning) return;

    _isRunning = false;
    _isPaused = false;

    ActuatorController::stopAllActuators();
    pidManager.stop();
    
    // Attendre que tous les actuateurs soient arrêtés
    unsigned long stopStartTime = millis();
    while (isAnyActuatorRunning() && millis() - stopStartTime < 5000) {
        // Attendre jusqu'à 5 secondes pour que tous les actuateurs s'arrêtent
        delay(100);
    }

    if (isAnyActuatorRunning()) {
        Logger::log(LogLevel::WARNING, F("Some actuators failed to stop within the timeout period"));
    }

    //Logger::log(LogLevel::INFO, F("Fermentation stopped"));
}

void FermentationProgram::updateVolume() {
    volumeManager.updateVolume();
}

void FermentationProgram::checkCompletion() {
  if (volumeManager.getCurrentVolume() >= volumeManager.getMaxAllowedVolume()) {
    stop();
    Logger::log(LogLevel::INFO, "Fermentation stopped: Volume limit reached");
    return;
  }
  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - startTime - totalPauseTime;
  if (elapsedTime >= static_cast<unsigned long>(duration) * 1000UL) {
    stop();
    Logger::log(LogLevel::INFO, "Fermentation completed. Elapsed time: " + String(elapsedTime/1000) + " s");
    Logger::log(LogLevel::INFO, F("Fermentation stopped: Duration exceeded"));
    Logger::log(LogLevel::INFO, "Duration set: " + String(duration/3600000.0, 2) + " hours");
    Logger::log(LogLevel::INFO, "Actual duration: " + String((currentTime - startTime) / 1000) + " seconds");
  }
}

void FermentationProgram::initializeStirringSpeed() {
    int minSpeed = max(MIN_STIRRING_SPEED, ActuatorController::getStirringMotorMinRPM());
    currentStirringSpeed = min(minSpeed, ActuatorController::getStirringMotorMaxRPM());
    pidManager.setMinStirringSpeed(currentStirringSpeed);
    ActuatorController::runActuator("stirringMotor", currentStirringSpeed, 0);  // 0 for continuous operation
    Logger::log(LogLevel::INFO, "Fermentation stirring speed initialized to: " + String(currentStirringSpeed) + " RPM");
}


// Dans parseCommand, simple ajout du nutrientDelay
void FermentationProgram::parseCommand(const String& command) {
    String params[9];
    int paramCount = 0;
    int lastIndex = command.indexOf(' ') + 1;
    
    while (lastIndex < command.length() && paramCount < 9) {
        if (command.charAt(lastIndex) == '"') {
            // Si on trouve un guillemet, chercher le guillemet fermant
            int endQuote = command.indexOf('"', lastIndex + 1);
            if (endQuote != -1) {
                // Extraire la chaîne sans les guillemets externes
                params[paramCount] = command.substring(lastIndex + 1, endQuote);
                lastIndex = endQuote + 2; // Sauter le guillemet et l'espace suivant
            } else {
                lastIndex = command.length();
            }
        } else {
            // Comportement normal pour les autres paramètres
            int spaceIndex = command.indexOf(' ', lastIndex);
            if (spaceIndex == -1) spaceIndex = command.length();
            params[paramCount] = command.substring(lastIndex, spaceIndex);
            lastIndex = spaceIndex + 1;
        }
        paramCount++;
    }
    
    if (paramCount >= 8) {
        float temp = params[0].toFloat();
        float ph = params[1].toFloat();
        float do_setpoint = params[2].toFloat();
        float nutrient_conc = params[3].toFloat();
        float base_conc = params[4].toFloat();
        float durationHours = params[5].toFloat();
        float nutrientDelay = params[6].toFloat();
        String experimentName = params[7];
        String comment = (paramCount > 8) ? params[8] : "";
        
        configure(temp, ph, do_setpoint, nutrient_conc, base_conc, 
                 durationHours, nutrientDelay, experimentName, comment);
        
        Logger::log(LogLevel::INFO, "ParseCommand - Duration parsed: " + String(durationHours) + " hours");
        Logger::log(LogLevel::INFO, "ParseCommand - Nutrient delay: " + String(nutrientDelay) + " hours");
    } else {
        Logger::log(LogLevel::ERROR, F("Invalid fermentation command format"));
    }
}

/*
void FermentationProgram::addNutrientsContinuously() {
    // Calculate elapsed time in hours
    float elapsedTime = (millis() - startTime) / 3600000.0;
    // Calculate total amount of nutrients to add based on current volume and desired concentration
    float totalNutrientToAdd = nutrientConc * volumeManager.getCurrentVolume();
    // Calculate the amount of nutrients to add now based on elapsed time
    float nutrientToAddNow = (totalNutrientToAdd / duration) * elapsedTime;
    // Check if it's safe to add the calculated amount of nutrients
    if (volumeManager.isSafeToAddVolume(nutrientToAddNow)) {
        // Get the maximum flow rate of the nutrient pump
        float maxFlowRate = ActuatorController::getPumpMaxFlowRate("nutrientPump");
        // Calculate how long the pump should run to add the desired amount of nutrients
        float pumpDuration = (nutrientToAddNow / maxFlowRate) * 60000; // Convert to milliseconds
        // Activate the nutrient pump
        ActuatorController::runActuator("nutrientPump", maxFlowRate, pumpDuration);
        // Record the volume change
        volumeManager.recordVolumeChange(nutrientToAddNow, "Nutrient");
        Logger::log(LogLevel::INFO, "Added nutrients: " + String(nutrientToAddNow) + " L");
    } else {
        //Logger::log(LogLevel::WARNING, "Cannot add nutrients: Volume limit reached");
        Logger::log(LogLevel::WARNING, F("Cannot add nutrients: Volume limit reached"));
    }
}
*/

void FermentationProgram::addNutrientsContinuouslyFixedRate(float fixedFlowRate) {
    unsigned long currentTime = millis();

    // Check if the program is still running
    if (!_isRunning || _isPaused) {
        if (ActuatorController::isActuatorRunning("nutrientPump")) {
            unsigned long runTime = currentTime - lastNutrientActivationTime;
            float addedVolume = (fixedFlowRate / 60.0) * (runTime / 1000.0);
            ActuatorController::stopActuator("nutrientPump");
            volumeManager.recordVolumeChange(addedVolume / 1000.0, "Nutrient"); // Convert to litres
            volumeManager.updateVolume();
            Logger::log(LogLevel::INFO, F("Nutrient pump stopped due to program stop/pause"));
            Logger::log(LogLevel::INFO, "Added volume: " + String(addedVolume, 3) + " ml");
        }
        return;
    }

    // Check whether we are currently adding nutrients
    if (ActuatorController::isActuatorRunning("nutrientPump")) {
        // Check if it's time to stop adding nutrients
        if (currentTime - lastNutrientActivationTime >= plannedNutrientActivationTime) {
            ActuatorController::stopActuator("nutrientPump");
            float addedVolume = (fixedFlowRate / 60.0) * (plannedNutrientActivationTime / 1000.0);
            volumeManager.recordVolumeChange(addedVolume / 1000.0, "Nutrient"); // Convert to litres
            volumeManager.updateVolume();
            Logger::log(LogLevel::INFO, F("Stopped adding nutrients"));
            Logger::log(LogLevel::INFO, "Added volume: " + String(addedVolume, 3) + " ml");
            Logger::log(LogLevel::INFO, "Current volume: " + String(volumeManager.getCurrentVolume(), 3) + " L");
            lastNutrientActivationTime = currentTime;
        }
        return;
    }

    // Check that we are still in the pause period
    if (currentTime - lastNutrientActivationTime < (plannedNutrientActivationTime + NUTRIENT_PAUSE_TIME)) {
        return;
    }

    // Checks before adding nutrients
    float currentVolume = volumeManager.getCurrentVolume();
    float maxAllowedVolume = volumeManager.getMaxAllowedVolume();
    if (currentVolume >= maxAllowedVolume) {
        stop();
        Logger::log(LogLevel::INFO, F("Fermentation stopped: Volume limit reached"));
        return;
    }

    if ((currentTime - startTime) > (duration * 1000UL)) {
        stop();
        Logger::log(LogLevel::INFO, F("Fermentation stopped: Duration exceeded"));
        return;
    }

    // Calculate the volume of nutrients to add
    float maxPossibleAddition = (fixedFlowRate / 60.0) * (NUTRIENT_ACTIVATION_TIME / 1000.0);
    float availableVolume = volumeManager.getAvailableVolume() * 1000; // Convert to ml
    float nutrientToAdd = min(maxPossibleAddition, availableVolume);

    // Start adding nutrients if there's room
    if (nutrientToAdd > 0) {
        Logger::log(LogLevel::INFO, "Starting nutrient addition: " + String(nutrientToAdd, 3) + " ml");
        plannedNutrientActivationTime = static_cast<unsigned long>((nutrientToAdd / maxPossibleAddition) * NUTRIENT_ACTIVATION_TIME);
        ActuatorController::runActuator("nutrientPump", fixedFlowRate, 0); // 0 for continuous duration
        lastNutrientActivationTime = currentTime;
        Logger::log(LogLevel::INFO, "Nutrient pump activated for planned duration: " + String(plannedNutrientActivationTime / 1000) + " s");
    } else {
        Logger::log(LogLevel::INFO, "No nutrients added: insufficient available volume");
    }
}

void FermentationProgram::setPIDEnabled(bool enabled) {
    isPIDEnabled = enabled;
    if (enabled) {
        pidManager.startTemperaturePID(tempSetpoint);
        pidManager.startPHPID(phSetpoint);
        pidManager.startDOPID(doSetpoint);
        Logger::log(LogLevel::INFO, F("PID control enabled"));
    } else {
        pidManager.stopTemperaturePID();
        pidManager.stopPHPID();
        pidManager.stopDOPID();
        Logger::log(LogLevel::INFO, F("PID control disabled"));
    }
}

void FermentationProgram::getParameters(JsonDocument& doc) const {
    doc["temperatureSetpoint"] = tempSetpoint;
    doc["phSetpoint"] = phSetpoint;
    doc["O2Setpoint"] = doSetpoint;
    doc["nutrientConcentration"] = nutrientConc;
    doc["baseConcentration"] = baseConc;
    doc["duration"] = getDuration();  // This will return the duration in hours
    doc["nutrientDelay"] = getNutrientStartDelay(); // Returns time in hours
    doc["experimentName"] = experimentName;
    doc["comment"] = comment;
}

bool FermentationProgram::isAnyActuatorRunning() const {
    return ActuatorController::isActuatorRunning("airPump") ||
           ActuatorController::isActuatorRunning("drainPump") ||
           ActuatorController::isActuatorRunning("samplePump") ||
           ActuatorController::isActuatorRunning("nutrientPump") ||
           ActuatorController::isActuatorRunning("basePump") ||
           ActuatorController::isActuatorRunning("stirringMotor") ||
           ActuatorController::isActuatorRunning("heatingPlate") ||
           ActuatorController::isActuatorRunning("ledGrowLight");
}
