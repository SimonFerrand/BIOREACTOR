#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 32

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup(void) {
 Serial.begin(115200);
 sensors.begin();
}

void loop(void) {
 sensors.requestTemperatures();
 float tempC = sensors.getTempCByIndex(0);
 
 if(tempC != DEVICE_DISCONNECTED_C) {
   Serial.print("Temperature: ");
   Serial.print(tempC);
   Serial.println("°C");
 }
 delay(1000);
}