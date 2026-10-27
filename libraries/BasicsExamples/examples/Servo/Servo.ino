// Servo SG90 con PWM (probado por board707 en Air103 a 350 Hz)
#define servo_pin PB0

void setAngle(uint8_t pin, uint8_t angle) {
  // duty de 40 a 230 (de 255) = de 0 a 180 grados a 350 Hz
  analogWrite(pin, map(angle, 0, 180, 40, 230));
}

void setup() {
  analogWriteFrequency(350);   // el SG90 admite 350 Hz
}

void loop() {
  for (uint8_t angle = 0; angle <= 180; angle++) {
    setAngle(servo_pin, angle);
    delay(10);
  }
  delay(200);
}
