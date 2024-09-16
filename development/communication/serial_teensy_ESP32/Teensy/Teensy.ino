// Teensy code for Serial communication test with ESP32
void setup() {
  // Initialize Serial communication with the computer (USB)
  Serial.begin(115200);

  // Initialize Serial1 communication with the ESP32
  Serial1.begin(9600); // Serial1 is on pins 0 (RX) and 1 (TX) on Teensy

  Serial.println("Teensy is ready for communication");
}

void loop() {
  // Check if data is available from the ESP32
  if (Serial1.available()) {
    String messageFromESP = Serial1.readStringUntil('\n');
    Serial.print("Message received from ESP32: ");
    Serial.println(messageFromESP);
  }

  // Send a message to the ESP32 every 2 seconds
  static unsigned long lastSendTime = 0;
  if (millis() - lastSendTime > 2000) {
    lastSendTime = millis();
    String message = "Hello from Teensy!";
    Serial1.println(message);
    Serial.print("Message sent to ESP32: ");
    Serial.println(message);
  }
}
