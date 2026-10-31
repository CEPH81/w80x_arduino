/*
 * Autoprueba del nucleo W80x (WM IoT SDK).
 * Abra el monitor serie a 115200: cada prueba muestra OK o FALLA.
 *
 * Opcional: un cable entre PUENTE_A y PUENTE_B prueba GPIO, interrupciones y PWM.
 * Opcional: un potenciometro en A0 (0 a 2,4 V como maximo) para ver el ADC.
 * En W800/W801/W802/Air101 tambien busca redes WiFi.
 */
#include <EEPROM.h>
#include <Preferences.h>
#include <HardwareTimer.h>
#include <Wire.h>
#if defined(CONFIG_CHIP_HAVE_WIFI) && CONFIG_CHIP_HAVE_WIFI
#include <WiFi.h>
#endif

#define PUENTE_A PB0   // salida (tiene PWM)
#define PUENTE_B PB1   // entrada

int ok = 0, fallos = 0;
void resultado(const char *nombre, bool bien, const char *detalle = "") {
  Serial.printf("%-28s %s  %s\n", nombre, bien ? "OK   " : "FALLA", detalle);
  if (bien) ok++; else fallos++;
}

volatile uint32_t flancos = 0;
void al_flanco() { flancos++; }
volatile uint32_t ticks = 0;
void al_timer() { ticks++; }
HardwareTimer tim(1);

void setup() {
  Serial.begin(115200);
  delay(1500);
  char d[96];
  Serial.printf("\n=== Autoprueba W80x  CPU %u MHz  memoria libre %u bytes ===\n", getCpuFrequencyMhz(), getFreeHeap());

  // 1. tiempos
  uint32_t m0 = millis(), u0 = micros();
  delay(1000);
  uint32_t dm = millis() - m0, du = micros() - u0;
  snprintf(d, sizeof(d), "millis %u ms, micros %u us", dm, du);
  resultado("delay(1000)", dm >= 999 && dm <= 1003 && du >= 999000 && du <= 1003000, d);
  u0 = micros(); delayMicroseconds(500); du = micros() - u0;
  snprintf(d, sizeof(d), "%u us", du);
  resultado("delayMicroseconds(500)", du >= 500 && du <= 520, d);
  bool mono = true; uint32_t ant = micros();
  for (int i = 0; i < 20000; i++) { uint32_t a = micros(); if (a < ant) mono = false; ant = a; }
  resultado("micros() siempre crece", mono);

  // 2. GPIO con el puente
  pinMode(PUENTE_A, OUTPUT); pinMode(PUENTE_B, INPUT);
  digitalWrite(PUENTE_A, HIGH); delay(2); int h = digitalRead(PUENTE_B);
  digitalWrite(PUENTE_A, LOW);  delay(2); int l = digitalRead(PUENTE_B);
  bool puente = (h == HIGH && l == LOW);
  resultado("GPIO (puente A-B)", puente, puente ? "" : "sin cable: se omiten GPIO, interrupcion y PWM");
  if (puente) {
    attachInterrupt(PUENTE_B, al_flanco, RISING);
    for (int i = 0; i < 100; i++) { digitalWrite(PUENTE_A, HIGH); delayMicroseconds(50); digitalWrite(PUENTE_A, LOW); delayMicroseconds(50); }
    delay(5);
    detachInterrupt(PUENTE_B);
    snprintf(d, sizeof(d), "%u de 100 flancos", flancos);
    resultado("attachInterrupt RISING", flancos == 100, d);

    analogWriteFrequency(1000);
    analogWrite(PUENTE_A, 64);          // 25 %
    delay(20);
    unsigned long alto = pulseIn(PUENTE_B, HIGH, 20000), bajo = pulseIn(PUENTE_B, LOW, 20000);
    snprintf(d, sizeof(d), "alto %lu us, bajo %lu us", alto, bajo);
    resultado("analogWrite 25% a 1 kHz", alto > 200 && alto < 300 && bajo > 700 && bajo < 800, d);
    analogWrite(PUENTE_A, 0);
  }

  // 3. ADC
  uint32_t mv = analogReadMilliVolts(A0);
  snprintf(d, sizeof(d), "A0 = %d (%u mV)", analogRead(A0), mv);
  resultado("analogRead(A0)", mv <= 2600, d);
  float t = temperatureRead();
  snprintf(d, sizeof(d), "%.1f C", t);
  resultado("temperatureRead()", t > -20 && t < 100, d);

  // 4. temporizador
  tim.configure(10, TIM_UNIT_MS, TIM_AUTORELOAD_PRELOAD_ENABLE, al_timer);
  tim.start(); delay(1000); tim.stop();
  snprintf(d, sizeof(d), "%u interrupciones (esperadas 100)", ticks);
  resultado("HardwareTimer 10 ms", ticks >= 98 && ticks <= 102, d);

  // 5. EEPROM y Preferences (se conservan entre reinicios)
  uint8_t arranques = EEPROM.read(0);
  if (arranques == 0xFF) arranques = 0;
  EEPROM.write(0, arranques + 1);
  resultado("EEPROM escribir/leer", EEPROM.read(0) == arranques + 1, "");
  Preferences p; p.begin("prueba");
  uint32_t n = p.getUInt("n", 0) + 1; p.putUInt("n", n);
  snprintf(d, sizeof(d), "arranque numero %u (reinicie: debe subir)", n);
  resultado("Preferences", p.getUInt("n", 0) == n, d);
  p.end();

  // 6. I2C: busca dispositivos (si hay alguno conectado)
  Wire.begin();
  int i2c = 0; uint8_t err5 = 0;
  for (uint8_t a = 1; a < 127; a++) { Wire.beginTransmission(a); uint8_t e = Wire.endTransmission(); if (e == 0) i2c++; if (e == 5) err5++; }
  snprintf(d, sizeof(d), "%d dispositivo(s) en SDA %d / SCL %d", i2c, SDA, SCL);
  resultado("Wire (escaner)", err5 == 0, err5 ? "bus bloqueado: faltan resistencias de 4,7k" : d);

  // 7. WiFi
#if defined(CONFIG_CHIP_HAVE_WIFI) && CONFIG_CHIP_HAVE_WIFI
  int redes = WiFi.scanNetworks();
  snprintf(d, sizeof(d), "%d redes, MAC %s", redes, WiFi.macAddress().c_str());
  resultado("WiFi.scanNetworks()", redes > 0, d);
  for (int i = 0; i < redes && i < 5; i++) Serial.printf("      %s (%d dBm)\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
#endif

  Serial.printf("=== %d OK, %d FALLA ===\n", ok, fallos);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // eco del monitor serie y parpadeo
  while (Serial.available()) Serial.write(Serial.read());
  digitalToggle(LED_BUILTIN);
  delay(fallos ? 100 : 500);   // rapido si algo fallo
}
