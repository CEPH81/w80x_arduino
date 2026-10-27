// Busca dispositivos en el bus I2C (SDA y SCL por defecto: ver pins_arduino.h)
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin();            // o Wire.begin(SDA, SCL) con otras patas
  Wire.setClock(100000);
}

void loop() {
  int encontrados = 0;
  Serial.println("Buscando...");
  for (uint8_t dir = 1; dir < 127; dir++) {
    Wire.beginTransmission(dir);
    uint8_t err = Wire.endTransmission();
    if (err == 0) {
      Serial.printf("Dispositivo en 0x%02X\n", dir);
      encontrados++;
    } else if (err == 5) {
      Serial.println("Bus bloqueado (tiempo agotado): revise las resistencias de 4,7k");
      break;
    }
  }
  Serial.printf("%d dispositivo(s)\n\n", encontrados);
  delay(5000);
}
