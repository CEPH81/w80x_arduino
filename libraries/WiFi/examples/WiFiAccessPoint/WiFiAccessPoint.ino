// Crea un punto de acceso WiFi con un servidor web (192.168.10.1)
#include <WiFi.h>

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  if (!WiFi.softAP("W80x-AP", "12345678")) {   // clave de 8 a 63 caracteres
    Serial.println("No se pudo crear el punto de acceso");
    return;
  }
  Serial.print("Punto de acceso listo, IP ");
  Serial.println(WiFi.softAPIP());
  server.begin();
}

void loop() {
  WiFiClient c = server.available();
  if (c) {
    // leer la peticion completa (hasta la linea vacia)
    unsigned long t0 = millis();
    while (c.connected() && millis() - t0 < 2000) {
      String l = c.readStringUntil('\n');
      if (l.length() <= 1) break;
    }
    c.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\nHola desde el W80x\r\n");
    c.stop();
  }
  static uint32_t t;
  if (millis() - t > 5000) {
    t = millis();
    Serial.printf("Clientes conectados: %u\n", WiFi.softAPgetStationNum());
  }
}
