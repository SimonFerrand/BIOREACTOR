#include "OxygenSensor.h"
#include "Logger.h"

OxygenSensor::OxygenSensor(HardwareSerial* serial, SensorInterface* tempSensor, const char* name)
    : _serial(serial), _tempSensor(tempSensor), _name(name) {}

void OxygenSensor::begin() {
    Logger::log(LogLevel::INFO, String(_name) + " initialized");
}

float OxygenSensor::readValue() {
    float temperature = _tempSensor->readValue();
    String response = sendCommand("O2:READ:" + String(temperature, 2));
    if (response.startsWith("O2:")) {
        float o2 = response.substring(3).toFloat();
        //Logger::log(LogLevel::INFO, String(_name) + " reading: " + String(o2));
        return o2;
    }
    Logger::log(LogLevel::ERROR, "Invalid O2 response: " + response);
    return -1;
}

void OxygenSensor::startCalibration(int points) {
    String response = sendCommand("O2:CAL:START:" + String(points));
    Logger::log(LogLevel::INFO, "O2 calibration start response: " + response);
}

void OxygenSensor::saveCalibrationPoint() {
    float temperature = _tempSensor->readValue();
    String response = sendCommand("O2:CAL:SAVE:" + String(temperature, 2));
    Logger::log(LogLevel::INFO, "O2 calibration save point response: " + response);
}

void OxygenSensor::finishCalibration() {
    String response = sendCommand("O2:CAL:FINISH");
    Logger::log(LogLevel::INFO, "O2 calibration finish response: " + response);
}

String OxygenSensor::getCalibrationStatus() {
    return sendCommand("O2:CAL:STATUS");
}

void OxygenSensor::resetCalibration() {
    String response = sendCommand("O2:CAL:RESET");
    Logger::log(LogLevel::INFO, "O2 calibration reset response: " + response);
}

String OxygenSensor::sendCommand(const String& cmd) {
    _serial->println(cmd);
    unsigned long startTime = millis();
    while (!_serial->available()) {
        if (millis() - startTime > 5000) {
            Logger::log(LogLevel::ERROR, "Timeout waiting for O2 sensor response");
            return "ERROR:TIMEOUT";
        }
    }
    return _serial->readStringUntil('\n');
}