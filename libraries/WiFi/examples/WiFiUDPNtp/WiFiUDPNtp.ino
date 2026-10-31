// Pide la hora a un servidor NTP por UDP
#include <WiFi.h>
#include <WiFiUdp.h>

const char *ssid     = "MiRed";
const char *password = "MiClave";
WiFiUDP udp;
uint8_t paquete[48];

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  udp.begin(2390);
}

void loop() {
  memset(paquete, 0, sizeof(paquete));
  paquete[0] = 0b11100011;   // version 4, modo cliente
  udp.beginPacket("pool.ntp.org", 123);
  udp.write(paquete, sizeof(paquete));
  udp.endPacket();
  delay(1000);
  if (udp.parsePacket() >= 48) {
    udp.read(paquete, 48);
    uint32_t segs = (uint32_t)paquete[40] << 24 | (uint32_t)paquete[41] << 16 | paquete[42] << 8 | paquete[43];
    uint32_t unix = segs - 2208988800UL;
    Serial.printf("Hora UTC: %02u:%02u:%02u\n", (unix % 86400) / 3600, (unix % 3600) / 60, unix % 60);
  } else {
    Serial.println("Sin respuesta del servidor NTP");
  }
  delay(10000);
}
