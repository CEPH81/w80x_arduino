// Guarda un byte y una estructura en la EEPROM emulada (se conservan al apagar)
#include <EEPROM.h>

struct Datos {
  uint32_t contador;
  float temperatura;
  char nombre[12];
};

void setup() {
  Serial.begin(115200);
  Serial.printf("EEPROM de %u bytes\n", EEPROM.length());

  uint8_t veces = EEPROM.read(0);
  if (veces == 0xFF) veces = 0;       // sin usar: vale 0xFF
  EEPROM.write(0, veces + 1);
  Serial.printf("Arranques: %u\n", veces + 1);

  Datos d;
  EEPROM.get(16, d);
  Serial.printf("Leido: contador=%u temperatura=%.2f nombre=%s\n", d.contador, d.temperatura, d.nombre);

  d.contador = veces + 1;
  d.temperatura = temperatureRead();
  strcpy(d.nombre, "W80x");
  EEPROM.put(16, d);
  EEPROM[100] += 1;                   // tambien como un arreglo
}

void loop() {}
