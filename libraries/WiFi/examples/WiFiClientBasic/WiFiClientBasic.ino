// Se conecta a una red WiFi y pide una pagina web por HTTP
#include <WiFi.h>

const char *ssid     = "MiRed";
const char *password = "MiClave";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.printf("MAC %s, senal %d dBm\n", WiFi.macAddress().c_str(), WiFi.RSSI());
}

void loop() {
  WiFiClient cliente;
  if (cliente.connect("example.com", 80)) {
    cliente.print("GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n");
    unsigned long t0 = millis();
    while (cliente.connected() && millis() - t0 < 5000) {
      while (cliente.available()) {
        Serial.write(cliente.read());
      }
      delay(1);
    }
    cliente.stop();
  } else {
    Serial.println("No se pudo conectar");
  }
  delay(30000);
}
