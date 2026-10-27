// Prueba del SPI: una MOSI con MISO (ver pins_arduino.h) y lo enviado vuelve.
#include <SPI.h>

void setup() {
  Serial.begin(115200);
  pinMode(SS, OUTPUT);
  digitalWrite(SS, HIGH);
  SPI.begin();
}

void loop() {
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(SS, LOW);
  uint8_t r = SPI.transfer(0xA5);
  uint8_t buf[4] = {1, 2, 3, 4};
  SPI.transfer(buf, sizeof(buf));
  digitalWrite(SS, HIGH);
  SPI.endTransaction();
  Serial.printf("enviado 0xA5, recibido 0x%02X; buf %d %d %d %d\n", r, buf[0], buf[1], buf[2], buf[3]);
  delay(1000);
}
