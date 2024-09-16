#include <HardwareSerial.h>

// Define serial communication pins
#define RXD2 19  // RX pin for ESP32
#define TXD2 18  // TX pin for ESP32

void setup() {
  // Initialize Serial communication with the computer (USB)
  Serial.begin(115200);
  
  // Initialize Serial communication with the Teensy
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); // Serial2 on ESP32 with custom RX/TX pins

  Serial.println("ESP32 is ready for communication");
}

void loop() {
  // Check if data is available from the Teensy
  if (Serial2.available()) {
    String messageFromTeensy = Serial2.readStringUntil('\n');
    Serial.print("Message received from Teensy: ");
    Serial.println(messageFromTeensy);
  }

  // Send a message to the Teensy every 2 seconds
  static unsigned long lastSendTime = 0;
  if (millis() - lastSendTime > 2000) {
    lastSendTime = millis();
    String message = "Hello from ESP32!";
    Serial2.println(message);
    Serial.print("Message sent to Teensy: ");
    Serial.println(message);
  }
}
