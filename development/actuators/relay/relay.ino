// Relay pin declaration
const int relaisPin = 1;

void setup() {
    // Set relayPin as output pin
  pinMode(relaisPin, OUTPUT);
}

void loop() {
  digitalWrite(relaisPin, HIGH); // Switch on relay
  delay(2000);                   
  digitalWrite(relaisPin, LOW);  // Switch relay off
  delay(2000);                  
}