/*
 * Temperatura interna del microcontrolador W80x.
 *
 * El sensor esta dentro del chip: mide la temperatura del silicio, que
 * suele estar unos grados por encima del ambiente (mas con el WiFi activo).
 * Sirve para vigilar que el chip no se caliente; no es un termometro de
 * ambiente.
 *
 * Muestra la temperatura actual, la media de los ultimos 10 valores,
 * la minima, la maxima y la tension de alimentacion del chip.
 */

const int N = 10;
float historial[N];
int indice = 0, cuenta = 0;
float minima = 1000, maxima = -1000;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Temperatura interna del W80x");
}

void loop() {
  float t = temperatureRead();          // grados C (promedio de 4 lecturas)
  if (isnan(t)) {
    Serial.println("No se pudo leer el sensor");
    delay(1000);
    return;
  }
  historial[indice] = t;
  indice = (indice + 1) % N;
  if (cuenta < N) cuenta++;
  float suma = 0;
  for (int i = 0; i < cuenta; i++) suma += historial[i];
  if (t < minima) minima = t;
  if (t > maxima) maxima = t;

  Serial.printf("Temperatura: %5.1f C   media(%d): %5.1f C   min %5.1f   max %5.1f   Vdd %u mV\n",
                t, cuenta, suma / cuenta, minima, maxima, analogReadVddMilliVolts());

  if (t > 85) Serial.println("  AVISO: el chip esta muy caliente");
  delay(1000);
}
