/*
 * ESP32-S3 LED Controller with LD24AJTA Driver Module
 * 
 * LD24AJTA Module (based on PT4115 IC):
 * - Input: 6-25V DC, Output: 30-900mA constant current
 * - PWM control range: 100Hz-20kHz
 * - Current range explanation: Module operates from 30mA minimum to 900mA maximum
 *   (NOT 0-900mA). For LED safety, we limit to LED's rated current.
 *   PWM calculation: (target_current - 30) / (900 - 30) * 255
 */

const int LED_PIN = 13;
const int LED_MAX_CURRENT_MA = 350;   // LED safe maximum current
const int MODULE_MAX_CURRENT_MA = 900; // LD24AJTA maximum current
const int MODULE_MIN_CURRENT_MA = 30;  // LD24AJTA minimum current

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, 0);
  
  Serial.println("ESP32-S3 + LD24AJTA - LED Controller");
  Serial.println("Enter 0-100 for percentage");
}

void loop() {
  if (Serial.available() > 0) {
    int percent = Serial.readStringUntil('\n').toInt();
    
    if (percent >= 0 && percent <= 100) {
      // Calculate PWM for current range 30-350mA
      float currentRatio = (float)(LED_MAX_CURRENT_MA - MODULE_MIN_CURRENT_MA) / 
                           (float)(MODULE_MAX_CURRENT_MA - MODULE_MIN_CURRENT_MA);
      int maxPWM = (int)(255 * currentRatio);
      int pwmValue = map(percent, 0, 100, 0, maxPWM);
      
      analogWrite(LED_PIN, pwmValue);
      
      float estimatedCurrent = MODULE_MIN_CURRENT_MA + 
                               (float)pwmValue / 255.0 * (MODULE_MAX_CURRENT_MA - MODULE_MIN_CURRENT_MA);
      
      Serial.print(percent);
      Serial.print("% -> PWM:");
      Serial.print(pwmValue);
      Serial.print(" -> ~");
      Serial.print(estimatedCurrent, 0);
      Serial.println("mA");
    }
  }
}