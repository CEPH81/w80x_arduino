/*
 * Serial_Puertos - los puertos serie de la placa elegida y una prueba de cada uno.
 *
 * Sirve para todas las placas: W800, W801, W802, W806, Air101 y Air103.
 * Al arrancar muestra en el Monitor serie (115200 baudios) los puertos de la placa
 * con sus patas, avisa si alguna pata es tambien un LED de la placa, y prueba
 * Serial1, Serial2... uno por uno enviando un texto de TX a RX.
 *
 * Para probar un puerto, una con un cable su TX con su RX (por ejemplo, Serial1:
 * PB6 con PB7) y pulse RST, o escriba "p" en el Monitor serie para repetir la prueba.
 *
 *   W800 / Air101 (chip W800): Serial, Serial1 ... Serial4
 *   W801, W802, W806, Air103:  Serial, Serial1 ... Serial5
 * Serial (TX PB19, RX PB20) es el del USB: se usa para el Monitor y para cargar.
 */

#if defined(ARDUINO_W800)
const char *placa = "W800 (HLK-W800-KIT)";
#elif defined(ARDUINO_W801)
const char *placa = "W801 (HLK-W801-KIT)";
#elif defined(ARDUINO_W802)
const char *placa = "W802";
#elif defined(ARDUINO_W806)
const char *placa = "W806 (HLK-W806-KIT)";
#elif defined(ARDUINO_AIR101)
const char *placa = "Air101";
#elif defined(ARDUINO_AIR103)
const char *placa = "Air103";
#else
const char *placa = "W80x";
#endif

struct Puerto {
  HardwareSerial *serie;
  const char *nombre;
  uint8_t tx, rx;
};

Puerto puertos[] = {
  {&Serial1, "Serial1", PIN_SERIAL1_TX, PIN_SERIAL1_RX},
  {&Serial2, "Serial2", PIN_SERIAL2_TX, PIN_SERIAL2_RX},
  {&Serial3, "Serial3", PIN_SERIAL3_TX, PIN_SERIAL3_RX},
  {&Serial4, "Serial4", PIN_SERIAL4_TX, PIN_SERIAL4_RX},
#if W80X_UART_COUNT > 5
  {&Serial5, "Serial5", PIN_SERIAL5_TX, PIN_SERIAL5_RX},
#endif
};
const int nPuertos = sizeof(puertos) / sizeof(puertos[0]);

// LED de la placa (variant.h); 0xFF = no hay
const uint8_t leds[] = {
#ifdef LED_BUILTIN_1
  LED_BUILTIN_1,
#endif
#ifdef LED_BUILTIN_2
  LED_BUILTIN_2,
#endif
#ifdef LED_BUILTIN_3
  LED_BUILTIN_3,
#endif
#ifdef LED_BUILTIN_4
  LED_BUILTIN_4,
#endif
#ifdef LED_BUILTIN_5
  LED_BUILTIN_5,
#endif
#ifdef LED_BUILTIN_6
  LED_BUILTIN_6,
#endif
#ifdef LED_BUILTIN_7
  LED_BUILTIN_7,
#endif
  0xFF
};

// nombre de la pata: 0..15 = PA0..PA15, 16.. = PB0..
String pata(uint8_t p) {
  return String(p < 16 ? "PA" : "PB") + String(p < 16 ? p : p - 16);
}

bool esLed(uint8_t p) {
  for (uint8_t l : leds)
    if (l == p) return true;
  return false;
}

void listar() {
  Serial.printf("\nPlaca %s: %d puertos serie\n", placa, W80X_UART_COUNT);
  Serial.printf("  %-8s TX %-5s RX %-5s (USB, Monitor serie)\n", "Serial",
                pata(PIN_SERIAL_TX).c_str(), pata(PIN_SERIAL_RX).c_str());
  for (int i = 0; i < nPuertos; i++) {
    Puerto &p = puertos[i];
    Serial.printf("  %-8s TX %-5s RX %-5s", p.nombre, pata(p.tx).c_str(), pata(p.rx).c_str());
    if (esLed(p.tx) || esLed(p.rx)) Serial.print(" (comparte pata con un LED de la placa)");
    Serial.println();
  }
}

void probar() {
  Serial.println("\nPrueba de lazo (cable de TX a RX en cada puerto que quiera probar):");
  for (int i = 0; i < nPuertos; i++) {
    Puerto &p = puertos[i];
    p.serie->begin(115200);
    delay(5);
    while (p.serie->available()) p.serie->read();
    const char *texto = "W80x-123";
    p.serie->print(texto);
    p.serie->flush();
    char eco[16] = {0};
    p.serie->setTimeout(50);
    p.serie->readBytes(eco, strlen(texto));
    bool ok = strcmp(eco, texto) == 0;
    Serial.printf("  %-8s %s-%s: %s\n", p.nombre, pata(p.tx).c_str(), pata(p.rx).c_str(),
                  ok ? "CORRECTA" : "sin respuesta (sin puente)");
    p.serie->end();  // deja las patas libres para la siguiente prueba
  }
  Serial.println("Escriba \"p\" para repetir la prueba.");
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== Serial_Puertos ===");
  listar();
  probar();
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'p' || c == 'P') probar();
  }
}
