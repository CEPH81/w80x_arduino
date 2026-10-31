// Busca las redes WiFi cercanas (W800, W801, W802, Air101)
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
}

void loop() {
  Serial.println("Buscando redes...");
  int n = WiFi.scanNetworks();
  if (n <= 0) {
    Serial.println("No se encontro ninguna red");
  } else {
    for (int i = 0; i < n; i++) {
      Serial.printf("%2d: %-32s %4d dBm  canal %2d  %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i),
                    WiFi.channel(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "abierta" : "con clave");
    }
  }
  WiFi.scanDelete();
  Serial.println();
  delay(10000);
}
