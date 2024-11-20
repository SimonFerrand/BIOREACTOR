// TestsProgram.cpp
#include "TestsProgram.h"

TestsProgram::TestsProgram(PIDManager& pidManager)
    : ProgramBase(),
      _currentTestType(TestType::INDIVIDUAL_ACTUATOR),
      _actuatorName(""),
      _testValue(0),
      _testDuration(0),
      _testStartTime(0),
      _currentActuatorTest(0),
      _pidManager(pidManager)
{
}

void TestsProgram::start(const String& command) {
    parseCommand(command);
    _isRunning = true;
    _isPaused = false;
    _testStartTime = millis();
    runTest();
    Logger::log(LogLevel::INFO, "Started test: " + getTestTypeName(_currentTestType));
}

void TestsProgram::update() {
    if (!_isRunning || _isPaused) return;

    unsigned long currentTime = millis();
    switch (_currentTestType) {
        case TestType::SENSORS:            
        case TestType::PID_TEMPERATURE:         
        case TestType::PID_PH:
        case TestType::PID_DISSOLVED_OXYGEN:
            updateContinuousTest();
            break;
        case TestType::INDIVIDUAL_ACTUATOR:
            if (currentTime - _testStartTime >= _testDuration) {
                stop();
            }
            break;
        case TestType::ALL_ACTUATORS:
            updateAllActuatorsTest();
            break;
    }
}

void TestsProgram::stop() {
    if (_isRunning) {
        switch (_currentTestType) {
            case TestType::INDIVIDUAL_ACTUATOR:
                ActuatorController::stopActuator(_actuatorName);
                break;
            case TestType::ALL_ACTUATORS:
                ActuatorController::stopAllActuators();
                break;
            case TestType::PID_TEMPERATURE:
            case TestType::PID_PH:
            case TestType::PID_DISSOLVED_OXYGEN:
                stopPIDTest();
                break;
            case TestType::SENSORS:
                // Sensor tests do not need to be stopped explicitly
                break;
            default:
                break;
        }
        _isRunning = false;
        _isPaused = false;
        Logger::log(LogLevel::INFO, "Test finished: " + getTestTypeName(_currentTestType));
    }
}

void TestsProgram::pause() {
    if (_isRunning && !_isPaused) {
        ActuatorController::stopAllActuators();
        _isPaused = true;
        Logger::log(LogLevel::INFO, "Test paused: " + getTestTypeName(_currentTestType));
    }
}

void TestsProgram::resume() {
    if (_isRunning && _isPaused) {
        // mettre le derniere acutateur lancé dans une variable & dernier program lancé + lancer le dernier actuateur... //A FAIRE
        // ...
        _isPaused = false;
        Logger::log(LogLevel::INFO, "Test resumed: " + getTestTypeName(_currentTestType));
    }
}

void TestsProgram::runTest() {
    switch (_currentTestType) {
        case TestType::INDIVIDUAL_ACTUATOR:
            runIndividualActuatorTest();
            break;
        case TestType::ALL_ACTUATORS:
            runAllActuatorsTest();
            break;
        case TestType::SENSORS:
            runSensorsTest();
            break;
        case TestType::PID_TEMPERATURE:
        case TestType::PID_PH:
        case TestType::PID_DISSOLVED_OXYGEN:
            runPIDTest();
            break;
    }
}

void TestsProgram::runIndividualActuatorTest() {
    ActuatorInterface* actuator = ActuatorController::findActuatorByName(_actuatorName);
    if (actuator) {
        ActuatorController::runActuator(_actuatorName, _testValue, 0); // 0 for continuous operation
        Logger::log(LogLevel::INFO, "Started individual actuator test: " + _actuatorName);
    } else {
        Logger::log(LogLevel::ERROR, "Actuator not found: " + _actuatorName);
        stop();
    }
}

void TestsProgram::runAllActuatorsTest() {
    _currentActuatorTest = 0;
    //Logger::log(LogLevel::INFO, "Started test of all actuators");
    Logger::log(LogLevel::INFO, F("Started test of all actuators"));
}

void TestsProgram::updateAllActuatorsTest() {
    unsigned long elapsedTime = millis() - _testStartTime;
    switch (_currentActuatorTest) {
        case 0:
            if (elapsedTime < 5000) {
                ActuatorController::runActuator("airPump", 50, 0);
            } else {
                ActuatorController::stopActuator("airPump");
                _currentActuatorTest++;
                //Logger::log(LogLevel::INFO, "Air Pump test completed");
                Logger::log(LogLevel::INFO, F("Air Pump test completed"));
            }
            break;
        case 1:
            if (elapsedTime < 10000) {
                ActuatorController::runActuator("drainPump", 80, 0);
            } else {
                ActuatorController::stopActuator("drainPump");
                _currentActuatorTest++;
                //Logger::log(LogLevel::INFO, "Drain Pump test completed");
                Logger::log(LogLevel::INFO, F("Drain Pump test completed"));
            }
            break;
        case 2:
            if (elapsedTime < 10000) {
                ActuatorController::runActuator("samplePump", 80, 0);
            } else {
                ActuatorController::stopActuator("samplePump");
                _currentActuatorTest++;
                //Logger::log(LogLevel::INFO, "Sample Pump test completed");
                Logger::log(LogLevel::INFO, F("Sample Pump test completed"));
            }
            break;
        case 3:
            if (elapsedTime < 15000) {
                ActuatorController::runActuator("stirringMotor", 1500, 0);
            } else {
                ActuatorController::stopActuator("stirringMotor");
                _currentActuatorTest++;
                //Logger::log(LogLevel::INFO, "Stirring Motor test completed");
                Logger::log(LogLevel::INFO, F("Stirring Motor test completed"));
            }
            break;
        case 4:
            if (elapsedTime < 20000) {
                ActuatorController::runActuator("nutrientPump", 50, 0);
            } else {
                ActuatorController::stopActuator("nutrientPump");
                _currentActuatorTest++;
                Logger::log(LogLevel::INFO, "Nutrient Pump test completed");
                Logger::log(LogLevel::INFO, F("Nutrient Pump test completed"));
            }
            break;
        case 5:
            if (elapsedTime < 25000) {
                ActuatorController::runActuator("basePump", 30, 0);
            } else {
                ActuatorController::stopActuator("basePump");
                _currentActuatorTest++;
                //Logger::log(LogLevel::INFO, "Base Pump test completed");
                Logger::log(LogLevel::INFO, F("Base Pump test completed"));
            }
            break;
        case 6:
            if (elapsedTime < 30000) {
                ActuatorController::runActuator("heatingPlate", 100, 0);
            } else {
                ActuatorController::stopActuator("heatingPlate");
                _currentActuatorTest++;
                //Logger::log(LogLevel::INFO, "Heating Plate test completed");
                Logger::log(LogLevel::INFO, F("Heating Plate test completed"));
            }
            break;
        case 7:
            if (elapsedTime < 35000) {
                ActuatorController::runActuator("ledGrowLight", 100, 0);
            } else {
                ActuatorController::stopActuator("ledGrowLight");
                _currentActuatorTest++;
                //Logger::log(LogLevel::INFO, "LED Grow Light test completed");
                Logger::log(LogLevel::INFO, F("LED Grow Light test completed"));
            }
            break;
        default:
            stop();
            break;
        case 8:
            if (elapsedTime < 10000) {
                ActuatorController::runActuator("fillPump", 80, 0);
            } else {
                ActuatorController::stopActuator("fillPump");
                _currentActuatorTest++;
                Logger::log(LogLevel::INFO, F("Fill Pump test completed"));
            }
            break;
    }
}

void TestsProgram::runSensorsTest() {
    //Logger::log(LogLevel::INFO, "Started sensors test");
    Logger::log(LogLevel::INFO, F("Started sensors test"));
    Logger::logSensorData();
}

void TestsProgram::runPIDTest() {
    switch (_currentTestType) {
        case TestType::PID_TEMPERATURE:
            _pidManager.startTemperaturePID(_testValue);
            break;
        case TestType::PID_PH:
            _pidManager.startPHPID(_testValue);
            break;
        case TestType::PID_DISSOLVED_OXYGEN:
            _pidManager.startDOPID(_testValue);
            break;
        default:
            //Logger::log(LogLevel::ERROR, "Invalid PID test type");
            Logger::log(LogLevel::ERROR, F("Invalid PID test type"));
            break;
    }
    Logger::log(LogLevel::INFO, "Started PID test: " + getTestTypeName(_currentTestType));
}

void TestsProgram::updateContinuousTest() {
    static unsigned long lastSensorLogTime = 0;
    const unsigned long sensorLogInterval = 15000; // Interval in milliseconds = 15 secondes
    unsigned long currentTime = millis();
    
    switch (_currentTestType) {
        case TestType::SENSORS:
            if (currentTime - lastSensorLogTime >= sensorLogInterval) {
                Logger::logSensorData();
                lastSensorLogTime = currentTime;
            }
            break;
        case TestType::PID_TEMPERATURE:
        case TestType::PID_PH:
        case TestType::PID_DISSOLVED_OXYGEN:
            _pidManager.updateAllPIDControllers();
            _pidManager.adjustPIDStirringSpeed();
            break;
        case TestType::INDIVIDUAL_ACTUATOR:
        case TestType::ALL_ACTUATORS:
            // These types of test do not require continuous updating
            break;
        default:
            break;
    }
}

String TestsProgram::getTestTypeName(TestType type) {
    switch (type) {
        case TestType::INDIVIDUAL_ACTUATOR: return "Individual Actuator";
        case TestType::ALL_ACTUATORS: return "All Actuators";
        case TestType::SENSORS: return "Sensors";
        case TestType::PID_TEMPERATURE: return "PID Temperature";
        case TestType::PID_PH: return "PID pH";
        case TestType::PID_DISSOLVED_OXYGEN: return "PID Dissolved Oxygen";
        default: return "Unknown";
    }
}

void TestsProgram::parseCommand(const String& command) {
    String cmd = command;
    cmd.toLowerCase();
    if (cmd == "tests") {
        _currentTestType = TestType::ALL_ACTUATORS;
    } else if (cmd == "test sensors") {
        _currentTestType = TestType::SENSORS;
    } else if (cmd.startsWith("test pid ")) {
        // Diviser la commande en parties
        int firstSpace = cmd.indexOf(' ', 9);  // Trouver l'espace après "test pid "
        if (firstSpace == -1) {
            Logger::log(LogLevel::ERROR, F("Invalid PID command format"));
            return;
        }
        
        String pidType = cmd.substring(9, firstSpace);  // Extraire uniquement le type
        _testValue = cmd.substring(firstSpace + 1).toFloat();  // Extraire la valeur
        
        if (pidType == "temp") {
            _currentTestType = TestType::PID_TEMPERATURE;
            Logger::log(LogLevel::INFO, "Starting temperature PID with setpoint: " + String(_testValue));
        } else if (pidType == "ph") {
            _currentTestType = TestType::PID_PH;
            Logger::log(LogLevel::INFO, "Starting pH PID with setpoint: " + String(_testValue));
        } else if (pidType == "do") {
            _currentTestType = TestType::PID_DISSOLVED_OXYGEN;
            Logger::log(LogLevel::INFO, "Starting DO PID with setpoint: " + String(_testValue));
        } else {
            Logger::log(LogLevel::ERROR, "Invalid PID type: " + pidType);
            _currentTestType = TestType::INDIVIDUAL_ACTUATOR;  // Fallback
            return;
        }
    } else if (cmd.startsWith("test ")) {
        _currentTestType = TestType::INDIVIDUAL_ACTUATOR;
        int firstSpace = cmd.indexOf(' ', 5);
        int secondSpace = cmd.indexOf(' ', firstSpace + 1);
        _actuatorName = command.substring(5, firstSpace); // Use original command to preserve case
        _testValue = command.substring(firstSpace + 1, secondSpace).toFloat();
        _testDuration = command.substring(secondSpace + 1).toInt() * 1000; // Convert to milliseconds
    } else {
        //Logger::log(LogLevel::ERROR, "Invalid test command");
        Logger::log(LogLevel::ERROR, F("Invalid test command"));
    }
}

void TestsProgram::stopPIDTest() {
    switch (_currentTestType) {
        case TestType::PID_TEMPERATURE:
            _pidManager.stopTemperaturePID();
            break;
        case TestType::PID_PH:
            _pidManager.stopPHPID();
            break;
        case TestType::PID_DISSOLVED_OXYGEN:
            _pidManager.stopDOPID();
            break;
        default:
            break;
    }
    ActuatorController::stopActuator("stirringMotor");
    Logger::log(LogLevel::INFO, "Stopped PID test: " + getTestTypeName(_currentTestType) + " and stirring motor");
}

void TestsProgram ::getParameters(JsonDocument& doc) const {
    doc["testVal"] = _testValue;
    doc["testDur"] = _testDuration;
}