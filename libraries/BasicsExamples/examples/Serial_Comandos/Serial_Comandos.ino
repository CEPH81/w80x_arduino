/*
 * Serial_Comandos - manejar la placa con ordenes de texto por el Monitor serie.
 *
 * Monitor serie a 115200 baudios con "Nueva linea". Ordenes:
 *   on / off / toggle   enciende, apaga o cambia el LED
 *   parpadeo <ms>       hace parpadear el LED (parpadeo 0 lo detiene)
 *   temp                temperatura interna del chip
 *   adc                 lectura de A0 en milivoltios (entrada de 0 a 2,4 V)
 *   info                frecuencia de la CPU, memoria libre y tiempo encendida
 *   ayuda               esta lista
 *
 * Usa serialEvent(), que el nucleo llama despues de cada loop() si hay datos en Serial.
 * Sirve para todas las placas (en la HLK-W806 el LED es el de PB0).
 */

const int led = LED_BUILTIN;
String orden;
bool ordenLista = false;
uint32_t periodo = 0, ultimo = 0;

void ayuda() {
  Serial.println("Ordenes: on, off, toggle, parpadeo <ms>, temp, adc, info, ayuda");
}

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  delay(300);
  Serial.println("\n=== Serial_Comandos ===");
  ayuda();
}

// el nucleo la llama sola cuando llegan datos por Serial
void serialEvent() {
  while (Serial.available() && !ordenLista) {
    char c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') ordenLista = true;
    else if (orden.length() < 40) orden += c;
  }
}

void ejecutar(String o) {
  o.trim();
  o.toLowerCase();
  if (o == "on") {
    periodo = 0; digitalWrite(led, HIGH); Serial.println("LED encendido");
  } else if (o == "off") {
    periodo = 0; digitalWrite(led, LOW); Serial.println("LED apagado");
  } else if (o == "toggle") {
    periodo = 0; digitalToggle(led); Serial.println("LED cambiado");
  } else if (o.startsWith("parpadeo")) {
    periodo = o.substring(8).toInt();
    if (periodo) Serial.printf("LED parpadeando cada %u ms\n", periodo);
    else Serial.println("Parpadeo detenido");
  } else if (o == "temp") {
    Serial.printf("Temperatura del chip: %.1f C\n", temperatureRead());
  } else if (o == "adc") {
    Serial.printf("A0 = %u mV\n", analogReadMilliVolts(A0));
  } else if (o == "info") {
    Serial.printf("CPU %u MHz, memoria libre %u bytes, encendida hace %u s\n",
                  getCpuFrequencyMhz(), getFreeHeap(), millis() / 1000);
  } else if (o == "ayuda" || o == "help" || o == "?") {
    ayuda();
  } else if (o.length()) {
    Serial.print("Orden desconocida: ");
    Serial.println(o);
    ayuda();
  }
}

void loop() {
  if (ordenLista) {
    ejecutar(orden);
    orden = "";
    ordenLista = false;
  }
  if (periodo && millis() - ultimo >= periodo) {
    ultimo = millis();
    digitalToggle(led);
  }
}
