#include "Logger.h"
#include <SoftwareSerial.h> // Ajoutez cette ligne

extern SoftwareSerial espSerial;

void logData(DCPump& airPump, DCPump& drainPump, PeristalticPump& nutrientPump, PeristalticPump& basePump,
             StirringMotor& stirringMotor, HeatingPlate& heatingPlate, LEDGrowLight& ledGrowLight,
             PT100Sensor& waterTempSensor, DS18B20TemperatureSensor& airTempSensor, PHSensor& phSensor,
             TurbiditySensor& turbiditySensor, OxygenSensor& oxygenSensor, AirFlowSensor& airFlowSensor,
             const String& experimentName, const String& comment) {

    String data = String("Experiment: ") + experimentName + ", Comment: " + comment + ", ";
    data += "Water_Temperature: " + String(waterTempSensor.readValue()) + " °C, ";
    data += "Air_Temperature: " + String(airTempSensor.readValue()) + " °C, ";
    data += "pH: " + String(phSensor.readValue()) + ", ";
    data += "Turbidity: " + String(turbiditySensor.readValue()) + ", ";
    data += "Dissolved_Oxygen: " + String(oxygenSensor.readValue()) + " mg/L, ";
    data += "Air_Flow: " + String(airFlowSensor.readValue()) + " L/min";

    espSerial.println(data);
}
