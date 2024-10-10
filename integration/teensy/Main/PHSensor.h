#ifndef PHSENSOR_H
#define PHSENSOR_H

#include "SensorInterface.h"
#include <Arduino.h>

class PHSensor : public SensorInterface {
public:
    PHSensor(HardwareSerial* serial, SensorInterface* tempSensor, const char* name);
    void begin() override;
    float readValue() override;
    const char* getName() const override { return _name; }
    void enterCalibration();
    void calibrate();
    void exitCalibration();

private:
    HardwareSerial* _serial;
    SensorInterface* _tempSensor;
    const char* _name;
    String sendCommand(const String& cmd);
};

#endif