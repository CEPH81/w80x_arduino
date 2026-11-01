/*
 * Serial_Eco - el puerto serie del USB (Serial) en lo basico.
 *
 * Abra el Monitor serie a 115200 baudios, escriba algo y pulse Enter:
 * la placa le devuelve el texto, cuantos caracteres tiene y el texto en mayusculas.
 *
 * Serial va por el conversor USB de la placa (TX PB19, RX PB20). Es el mismo
 * puerto que se usa para cargar el programa.
 * Sirve para todas las placas: W800, W801, W802, W806, Air101 y Air103.
 */

String linea;

void setup() {
  Serial.begin(115200);
  delay(300);  // tiempo para que se abra el Monitor serie
  Serial.println();
  Serial.println("=== Serial_Eco ===");
  Serial.printf("CPU a %u MHz, memoria libre %u bytes\n", getCpuFrequencyMhz(), getFreeHeap());
  Serial.println("Escriba un texto y pulse Enter (Monitor serie con \"Nueva linea\").");
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r') continue;          // se ignora el retorno de carro
    if (c == '\n') {                  // fin de linea: se responde
      Serial.print("Recibido: \"");
      Serial.print(linea);
      Serial.printf("\" (%u caracteres)\n", linea.length());
      String may = linea;
      may.toUpperCase();
      Serial.print("En mayusculas: ");
      Serial.println(may);
      linea = "";
    } else if (linea.length() < 128) {
      linea += c;
    }
  }
}
