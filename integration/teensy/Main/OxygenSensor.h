#ifndef OXYGENSENSOR_H
#define OXYGENSENSOR_H

#include "SensorInterface.h"
#include <Arduino.h>

class OxygenSensor : public SensorInterface {
public:
    OxygenSensor(HardwareSerial* serial, SensorInterface* tempSensor, const char* name);
    void begin() override;
    float readValue() override;
    const char* getName() const override { return _name; }
    
    void startCalibration();
    void calibrateZero();
    void calibrateSatLow();
    void calibrateSatHigh();
    void resetCalibration();
    void getCalibrationStatus();

private:
    HardwareSerial* _serial;
    SensorInterface* _tempSensor;
    const char* _name;
    String sendCommand(const String& cmd);
};

#endif