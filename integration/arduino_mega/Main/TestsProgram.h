// TestsProgram.h
#ifndef TESTS_PROGRAM_H
#define TESTS_PROGRAM_H

#include "ProgramBase.h"
#include "ActuatorController.h"
#include "SensorController.h"
#include "PIDManager.h"
#include "Logger.h"

class TestsProgram : public ProgramBase {
public:
    enum class TestType {
        INDIVIDUAL_ACTUATOR,
        ALL_ACTUATORS,
        SENSORS,
        PID_TEMPERATURE,
        PID_PH,
        PID_DISSOLVED_OXYGEN
    };
    TestsProgram(PIDManager& pidManager);
    void start(const String& command) override;
    void update() override;
    void stop() override;
    void stopPIDTest();
    void pause() override;
    void resume() override;
    bool isRunning() const override { return _isRunning; }
    bool isPaused() const override { return _isPaused; }
    String getName() const override { return "Tests"; }
    void parseCommand(const String& command) override;

private:
    TestType _currentTestType;
    String _actuatorName;
    float _testValue;
    unsigned long _testDuration;
    unsigned long _testStartTime;
    int _currentActuatorTest;
    PIDManager& _pidManager;
    

    void runTest();
    void runIndividualActuatorTest();
    void runAllActuatorsTest();
    void updateAllActuatorsTest();
    void runSensorsTest();
    void runPIDTest();
    void updateContinuousTest();
    String getTestTypeName(TestType type);
};

#endif // TESTS_PROGRAM_H