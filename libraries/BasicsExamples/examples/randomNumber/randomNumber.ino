// Numeros aleatorios: generador por hardware (TRNG) y random() de Arduino
void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.printf("aleatorio por hardware: %u\n", w80x_random());
  Serial.printf("random(0, 1000): %ld\n", random(0, 1000));  // tambien usa el TRNG
  delay(1000);
}
