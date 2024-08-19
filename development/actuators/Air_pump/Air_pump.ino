// This Arduino sketch configures PWM output to control a DC motor pump using a PW1VA01 module, which converts PWM signals to a 0-5V output.
// The PW1VA01 module is powered by 12V and connects to the Arduino via pin 5.
// The 0-5V output from the PW1VA01 controls a DC motor speed controller, which is connected to a 12V DC air pump.

void setup() {
  // Initialize serial communication at 115200 baud rate
  Serial.begin(115200);

  // Configure pin 5 as PWM output
  pinMode(9, OUTPUT);

  // Set PWM value to 100%
  int percentage = 70;
  int pwmValue = map(percentage, 0, 100, 0, 255);  // Convert from 0-100% to 0-255
  analogWrite(5, pwmValue);
  Serial.print("PWM set to %: ");
  Serial.println(pwmValue);
}

void loop() {
  // Nothing to do here
}