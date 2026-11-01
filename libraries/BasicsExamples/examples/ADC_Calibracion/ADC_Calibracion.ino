/*
 * Calibracion del ADC del W80x (entrada de 0 a 2,4 V COMO MAXIMO).
 *
 * El SDK ya aplica la calibracion de fabrica si el chip la trae. Con este
 * programa se anade una calibracion propia de dos puntos, que se guarda en
 * la flash y se aplica a analogRead() y analogReadMilliVolts() en todos los
 * programas siguientes.
 *
 * Material: un multimetro y dos tensiones estables en A0, por ejemplo:
 *   punto 1: unos 200 mV (o GND, 0 mV)
 *   punto 2: unos 2000 mV (divisor desde 3,3 V: 10k arriba y 15k abajo = 1,98 V)
 * Nunca pase de 2,4 V en la entrada.
 *
 * Uso, en el monitor serie (115200, "Nueva linea"):
 *   l          lee A0 (con y sin calibracion propia)
 *   1 <mV>     punto 1: ponga la tension, midala con el multimetro y escriba "1 198"
 *   2 <mV>     punto 2: igual, "2 1985"
 *   g          calcula y guarda la calibracion
 *   r          borra la calibracion propia (queda la de fabrica)
 */

const uint8_t PATA = A0;
uint32_t leido1 = 0, real1 = 0, leido2 = 0, real2 = 0;

uint32_t leerPromedio() {
  analogReadAveraging(32);                    // 32 muestras por lectura
  uint32_t v = analogReadUncalibratedMilliVolts(PATA);
  analogReadAveraging(1);
  return v;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Calibracion del ADC (maximo 2,4 V en la entrada)");
  Serial.printf("Calibracion de fabrica: %s\n", analogFactoryCalibrated() ? "si" : "no (se usa la formula del SDK)");
  Serial.println("Comandos: l, 1 <mV>, 2 <mV>, g, r");
}

void loop() {
  if (!Serial.available()) return;
  String linea = Serial.readStringUntil('\n');
  linea.trim();
  if (linea.length() == 0) return;
  char cmd = linea[0];
  long valor = linea.length() > 2 ? linea.substring(2).toInt() : 0;

  if (cmd == 'l') {
    unsigned long t0 = micros();
    uint32_t mv = analogReadMilliVolts(PATA);
    unsigned long dt = micros() - t0;
    Serial.printf("A0: %u mV calibrado, %u mV sin calibracion propia, analogRead=%d (lectura en %lu us)\n",
                  mv, leerPromedio(), analogRead(PATA), dt);
  } else if (cmd == '1' || cmd == '2') {
    uint32_t l = leerPromedio();
    if (cmd == '1') { leido1 = l; real1 = valor; } else { leido2 = l; real2 = valor; }
    Serial.printf("Punto %c: leido %u mV, real %ld mV\n", cmd, l, valor);
  } else if (cmd == 'g') {
    if (leido1 == 0 && leido2 == 0) {
      Serial.println("Primero tome los puntos 1 y 2");
    } else if (analogCalibrate(leido1, real1, leido2, real2)) {
      Serial.println("Calibracion guardada. Compruebe con 'l'.");
    } else {
      Serial.println("No se pudo: los puntos son iguales o el error pasa del 50 %");
    }
  } else if (cmd == 'r') {
    analogCalibrationReset();
    Serial.println("Calibracion propia borrada");
  }
}
