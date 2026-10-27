/*
 * Lectura analogica en W80x: A1..A4 (PA1..PA4; A0 = A1). En W800/Air101: A0 (PA1) y A1 (PA4).
 * Entrada de 0 a 2,4 V.
 *   analogRead()            -> 0..4095 (12 bits; analogReadResolution() lo cambia)
 *   analogReadMilliVolts()  -> milivoltios (calibracion de fabrica)
 */
void setup() {
  Serial.begin(115200);
}

void loop() {
  int crudo = analogRead(A0);
  uint32_t mv = analogReadMilliVolts(A0);
  Serial.printf("ADC: %d  (%u mV)  temperatura interna %.1f C\n", crudo, mv, temperatureRead());
  delay(1000);
}
