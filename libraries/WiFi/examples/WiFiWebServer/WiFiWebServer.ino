// Servidor web: muestra el valor de A0 y enciende/apaga el LED (/on, /off)
#include <WiFi.h>

const char *ssid     = "MiRed";
const char *password = "MiClave";
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.print("Abra http://");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  WiFiClient c = server.available();
  if (!c) return;
  String linea = c.readStringUntil('\n');   // "GET /on HTTP/1.1"
  while (c.available()) c.read();           // resto de la peticion
  if (linea.indexOf("GET /on") >= 0) digitalWrite(LED_BUILTIN, LOW);
  if (linea.indexOf("GET /off") >= 0) digitalWrite(LED_BUILTIN, HIGH);
  c.print("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nConnection: close\r\n\r\n");
  c.printf("<h1>W80x</h1><p>A0 = %d</p><p><a href=/on>LED encendido</a> | <a href=/off>LED apagado</a></p>",
           analogRead(A0));
  c.stop();
}
