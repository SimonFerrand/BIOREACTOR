#include "PHSensor.h"
#include "Logger.h"

PHSensor::PHSensor(HardwareSerial* serial, SensorInterface* tempSensor, const char* name)
    : _serial(serial), _tempSensor(tempSensor), _name(name) {}

void PHSensor::begin() {
    Logger::log(LogLevel::INFO, String(_name) + " initialized");
}

float PHSensor::readValue() {
    float temperature = _tempSensor->readValue();
    String response = sendCommand("PH:READ:" + String(temperature, 2));
    if (response.startsWith("PH:")) {
        float pH = response.substring(3).toFloat();
        return pH;
    }
    Logger::log(LogLevel::ERROR, "Invalid pH response: " + response);
    return -1;
}

void PHSensor::enterCalibration() {
    float temperature = _tempSensor->readValue();
    String response = sendCommand("PH:CAL:ENTERPH:" + String(temperature, 2));
    // La réponse sera "ENTERPH:OK" ou "ENTERPH:INVALID_CMD"
    Logger::log(LogLevel::INFO, "pH calibration response: " + response);
    Logger::log(LogLevel::INFO, F(". Place probe in pH 7.0 or pH 4.0 buffer solution"));
    Logger::log(LogLevel::INFO, F(". Type 'ph CALPH' when ready"));
}

void PHSensor::calibrate() {
    float temperature = _tempSensor->readValue();
    String response = sendCommand("PH:CAL:CALPH:" + String(temperature, 2));
    Logger::log(LogLevel::INFO, "pH calibration response: " + response);
    Logger::log(LogLevel::INFO, F("4. Type 'ph EXITPH' to save and finish"));
}

void PHSensor::exitCalibration() {
    float temperature = _tempSensor->readValue();
    String response = sendCommand("PH:CAL:EXITPH:" + String(temperature, 2));
    Logger::log(LogLevel::INFO, "pH calibration exit response: " + response);
    Logger::log(LogLevel::INFO, F("Calibration saved"));
}

String PHSensor::sendCommand(const String& cmd) {
    _serial->println(cmd);
    unsigned long startTime = millis();
    while (!_serial->available()) {
        if (millis() - startTime > 5000) {
            Logger::log(LogLevel::ERROR, "Timeout waiting for pH sensor response");
            return "ERROR:TIMEOUT";
        }
    }
    return _serial->readStringUntil('\n');
}