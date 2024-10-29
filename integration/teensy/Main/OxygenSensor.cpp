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

void OxygenSensor::startCalibration() {
    String response = sendCommand("O2:CAL:START");
    Logger::log(LogLevel::INFO, "O2 calibration response: " + response);
    Logger::log(LogLevel::INFO, F("Starting O2 calibration"));
    Logger::log(LogLevel::INFO, F("1. Prepare zero solution (2g Na2S2O3 in 100mL)"));
    Logger::log(LogLevel::INFO, F("   Wait 1 hour and type 'o2 cal zero' when ready"));
}

void OxygenSensor::calibrateZero() {
    float temperature = _tempSensor->readValue();
    String response = sendCommand("O2:CAL:ZERO:" + String(temperature, 1));
    Logger::log(LogLevel::INFO, "O2 zero calibration response: " + response);
    Logger::log(LogLevel::INFO, F("2. Prepare room temperature water (20-25°C)"));
    Logger::log(LogLevel::INFO, F("   Aerate for 15min minimum"));
    Logger::log(LogLevel::INFO, F("   Type 'o2 cal low' when ready"));
}

void OxygenSensor::calibrateSatLow() {
    float temperature = _tempSensor->readValue();
    String response = sendCommand("O2:CAL:SAT_LOW:" + String(temperature, 1));
    Logger::log(LogLevel::INFO, "O2 low saturation calibration response: " + response);
    Logger::log(LogLevel::INFO, F("3. Heat water to ~35°C while aerating"));
    Logger::log(LogLevel::INFO, F("   Type 'o2 cal high' when ready"));
}

void OxygenSensor::calibrateSatHigh() {
    float temperature = _tempSensor->readValue();
    String response = sendCommand("O2:CAL:SAT_HIGH:" + String(temperature, 1));
    Logger::log(LogLevel::INFO, "O2 high saturation calibration response: " + response);
    Logger::log(LogLevel::INFO, F("Calibration completed!"));
}

void OxygenSensor::getCalibrationStatus() {
    String response = sendCommand("O2:CAL:STATUS");
    Logger::log(LogLevel::INFO, "O2 calibration status: " + response);
}
void OxygenSensor::resetCalibration() {
    String response = sendCommand("O2:CAL:RESET");
    Logger::log(LogLevel::INFO, "O2 calibration reset response: " + response);
}

String OxygenSensor::sendCommand(const String& cmd) {
    //Logger::log(LogLevel::INFO, "Sending command to Arduino: " + cmd);
    _serial->println(cmd);
    
    unsigned long startTime = millis();
    while (!_serial->available()) {
        if (millis() - startTime > 5000) {
            Logger::log(LogLevel::ERROR, "Timeout waiting for O2 sensor response");
            return "ERROR:TIMEOUT";
        }
    }
    String response = _serial->readStringUntil('\n');
    response.trim();
    //Logger::log(LogLevel::INFO, "Received response from Arduino: " + response);
    return response;
}