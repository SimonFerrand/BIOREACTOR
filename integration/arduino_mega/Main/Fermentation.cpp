#include "Fermentation.h"
#include "Logger.h"
#include "PIDControllers.h"

extern SoftwareSerial espSerial;

void runFermentation(DCPump& airPump, DCPump& drainPump, PeristalticPump& nutrientPump, PeristalticPump& basePump,
                     StirringMotor& stirringMotor, HeatingPlate& heatingPlate, LEDGrowLight& ledGrowLight,
                     PT100Sensor& waterTempSensor, DS18B20TemperatureSensor& airTempSensor, PHSensor& phSensor,
                     TurbiditySensor& turbiditySensor, OxygenSensor& oxygenSensor, AirFlowSensor& airFlowSensor,
                     float tempSetpointVal, float phSetpointVal, float doSetpointVal, float nutrientConc, float baseConc,
                     float stirSpeed, int duration, const String& experimentName, const String& comment) {

    tempSetpoint = tempSetpointVal;
    phSetpoint = phSetpointVal;
    doSetpoint = doSetpointVal;

    unsigned long startTime = millis();
    while (millis() - startTime < duration * 1000) {
        float waterTemp = waterTempSensor.readValue();
        float ph = phSensor.readValue();
        float doValue = oxygenSensor.readValue();

        tempInput = waterTemp;
        phInput = ph;
        doInput = doValue;

        tempPID.Compute();
        phPID.Compute();
        doPID.Compute();

        heatingPlate.control(tempOutput > 0, tempOutput);
        nutrientPump.control(phOutput > 0, nutrientConc * phOutput / 100.0);
        airPump.control(doOutput > 0, doOutput);

        logData(airPump, drainPump, nutrientPump, basePump, stirringMotor, heatingPlate, ledGrowLight,
                waterTempSensor, airTempSensor, phSensor, turbiditySensor, oxygenSensor, airFlowSensor,
                experimentName, comment);

        delay(1000); // Log data every second
    }
}
