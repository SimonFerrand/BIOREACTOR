#include <SerialTransfer.h>

SerialTransfer myTransfer;

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);
  myTransfer.begin(Serial1);
  Serial.println("Arduino Mega Ready");
}

void loop() {
  if (myTransfer.available()) {
    char message[50];
    myTransfer.rxObj(message, sizeof(message));
    Serial.print("Received from ESP32: ");
    Serial.println(message);
  }

  char msg[] = "Hello from Arduino Mega";
  myTransfer.txObj(msg, sizeof(msg));
  myTransfer.sendData(sizeof(msg));

  delay(5000);
}
