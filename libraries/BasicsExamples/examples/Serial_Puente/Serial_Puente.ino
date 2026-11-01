/*
 * Serial_Puente - un segundo puerto serie (Serial1) unido al del USB.
 *
 * Lo que escriba en el Monitor serie sale por Serial1 y lo que llegue por Serial1
 * aparece en el Monitor. Sirve para hablar con un modulo serie (GPS, Bluetooth HC-05,
 * otro microcontrolador...) desde el PC.
 *
 * Conexion (Serial1, patas por defecto):
 *   PB6 = TX1  ->  RX del modulo
 *   PB7 = RX1  <-  TX del modulo
 *   GND        --  GND del modulo        (senales de 3,3 V)
 *
 * Prueba sin modulo: una PB6 con PB7 con un cable. Al arrancar se hace una prueba
 * de lazo y todo lo que escriba le vuelve como "[modulo]".
 *
 * Serial1 esta en PB6/PB7 en todas las placas (W800, W801, W802, W806, Air101, Air103).
 * Para usar otro puerto cambie PUERTO (por ejemplo Serial3); el ejemplo Serial_Puertos
 * muestra las patas de cada puerto en su placa. Tambien se pueden elegir otras patas:
 *   Serial1.begin(9600, SERIAL_8N1, rx, tx);
 */

#define VELOCIDAD_MODULO 9600   // la del modulo conectado (HC-05: 9600, GPS: 9600...)
#define PUERTO  Serial1          // puerto hacia el modulo
#define PUERTO_TX PIN_SERIAL1_TX // sus patas (solo para mostrarlas)
#define PUERTO_RX PIN_SERIAL1_RX

String pata(uint8_t p) { return String(p < 16 ? "PA" : "PB") + String(p < 16 ? p : p - 16); }

void setup() {
  Serial.begin(115200);
  PUERTO.begin(VELOCIDAD_MODULO);
  delay(300);
  Serial.println("\n=== Serial_Puente ===");
  Serial.printf("Puerto a %d baudios (TX %s, RX %s)\n", VELOCIDAD_MODULO,
                pata(PUERTO_TX).c_str(), pata(PUERTO_RX).c_str());

  // prueba de lazo: si PB6 y PB7 estan unidas, el texto vuelve
  while (PUERTO.available()) PUERTO.read();
  PUERTO.print("hola");
  PUERTO.flush();
  delay(20);
  char eco[8] = {0};
  PUERTO.readBytes(eco, 4);
  if (strcmp(eco, "hola") == 0) Serial.println("Prueba de lazo TX-RX: correcta");
  else Serial.println("Sin lazo TX-RX (normal si hay un modulo conectado)");
  Serial.println("Escriba en el Monitor serie para enviar al modulo.");
}

void loop() {
  // PC -> modulo
  while (Serial.available()) PUERTO.write(Serial.read());

  // modulo -> PC, por lineas para marcarlas
  static bool inicio = true;
  while (PUERTO.available()) {
    char c = PUERTO.read();
    if (inicio) { Serial.print("[modulo] "); inicio = false; }
    Serial.write(c);
    if (c == '\n') inicio = true;
  }
}
