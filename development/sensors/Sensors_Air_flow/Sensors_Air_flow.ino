/*
  Instructions to connect the 3-wire air flow meter (YF-S401) to the Arduino:
  https://www.epitran.it/ebayDrive/datasheet/YF-S401.pdf

  1. Connections for the 3-wire air flow meter to Arduino:
     - Red Wire (Power): Connect to 5V on the Arduino.
     - Black Wire (Ground): Connect to GND on the Arduino.
     - Yellow Wire (Signal): Connect to a digital input pin (e.g., pin 2) on the Arduino.

     On the Arduino Mega, external interrupts are only available on certain specific pins. The pins that support interrupts are: 2, 3, 18, 19, 20, 21 (interrupts 0, 1, 5, 4, 3, 2 respectively).
  
  2. The air flow meter provides a pulsed output proportional to the flow rate.
     This code will count the pulses over a specific time period to calculate the flow rate.

  Flow rate range: 1 to 5 liters per minute.
  Measurement error: ±2%.
  Pulses per liter: 5880 pulses/L. (=>Frequency: F = 98 * Q (L / Min)). F = (98 * Q) ± 2% avec Q = L / MIN
  Pressure conversion: 0.8 MPa = 8 bar


  
*/

const int flowMeterPin = 2; // Pin connected to the flow meter's signal wire
volatile unsigned long pulseCount = 0;
unsigned long lastTime = 0;
const unsigned long interval = 1000; // Interval to calculate flow rate (in milliseconds)

float flowRate = 0.0; // Flow rate in liters per minute (L/min)
const float pulsesPerLiter = 5880.0; // Pulses per liter as per the sensor's specification // 98

// Interrupt service routine to count pulses
void countPulses() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);  // Start the serial communication at 115200 baud rate
  pinMode(flowMeterPin, INPUT_PULLUP); // Set the flow meter pin as input with internal pull-up resistor
  attachInterrupt(digitalPinToInterrupt(flowMeterPin), countPulses, FALLING); // Attach interrupt to count pulses
}

void loop() {
  unsigned long currentTime = millis();
  
  // Calculate flow rate every 'interval' milliseconds
  if (currentTime - lastTime >= interval) {
    noInterrupts(); // Disable interrupts while calculating
    unsigned long pulses = pulseCount;
    pulseCount = 0; // Reset pulse count
    interrupts(); // Enable interrupts

    // Flow rate calculation
    float flowRate = (pulses / pulsesPerLiter) * 60.0; // Convert pulses to liters per minute
    
    //Serial.println(pulses);
    Serial.print("Flow Rate: ");
    Serial.print(flowRate);
    Serial.println(" L/min");

    lastTime = currentTime;
  }
}