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
 * de lazo y todo lo que escriba le vuelve como "[Serial1]".
 *
 * Otros puertos (ver variants/<placa>/pins_arduino.h):
 *   Serial2 TX PB2  RX PB3   (en la HLK-W806, PB2 es un LED)
 *   Serial3 TX PA5  RX PA6
 *   Serial4 TX PA8  RX PA9
 *   Serial5 TX PB18 RX PB17
 * Tambien se pueden elegir otras patas: Serial1.begin(9600, SERIAL_8N1, rx, tx);
 */

#define VELOCIDAD_MODULO 9600   // la del modulo conectado (HC-05: 9600, GPS: 9600...)

void setup() {
  Serial.begin(115200);
  Serial1.begin(VELOCIDAD_MODULO);   // TX PB6, RX PB7
  delay(300);
  Serial.println("\n=== Serial_Puente ===");
  Serial.printf("Serial1 a %d baudios (TX PB6, RX PB7)\n", VELOCIDAD_MODULO);

  // prueba de lazo: si PB6 y PB7 estan unidas, el texto vuelve
  while (Serial1.available()) Serial1.read();
  Serial1.print("hola");
  Serial1.flush();
  delay(20);
  char eco[8] = {0};
  Serial1.readBytes(eco, 4);
  if (strcmp(eco, "hola") == 0) Serial.println("Prueba de lazo PB6-PB7: correcta");
  else Serial.println("Sin lazo PB6-PB7 (normal si hay un modulo conectado)");
  Serial.println("Escriba en el Monitor serie para enviar por Serial1.");
}

void loop() {
  // PC -> modulo
  while (Serial.available()) Serial1.write(Serial.read());

  // modulo -> PC, por lineas para marcarlas
  static bool inicio = true;
  while (Serial1.available()) {
    char c = Serial1.read();
    if (inicio) { Serial.print("[Serial1] "); inicio = false; }
    Serial.write(c);
    if (c == '\n') inicio = true;
  }
}
