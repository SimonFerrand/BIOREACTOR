void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("ESP32 Ready");
  Serial2.begin(9600, SERIAL_8N1, 18, 19);
  Serial2.setRxBufferSize(256);
}

void loop() {
  static String receivedData = "";

  while (Serial2.available()) {
    char incomingChar = Serial2.read();
    receivedData += incomingChar;

    if (incomingChar == '\n') {
      Serial.print("Received from Arduino Mega: ");
      Serial.print(receivedData);
      receivedData = "";
    }
  }

  delay(500);
  Serial2.println("Ping from ESP32\n");

  int bufferSize = Serial2.availableForWrite();
  int percentageUsed = (100 * (256 - bufferSize)) / 256;
  Serial.print("Buffer used: ");
  Serial.print(percentageUsed);
  Serial.println("%");
}
