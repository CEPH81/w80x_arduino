// Cuenta los reinicios de la placa y lo guarda en la flash
#include <Preferences.h>

Preferences prefs;

void setup() {
  Serial.begin(115200);
  prefs.begin("contador");
  uint32_t n = prefs.getUInt("reinicios", 0) + 1;
  prefs.putUInt("reinicios", n);
  prefs.end();
  Serial.printf("La placa se ha iniciado %u veces\n", n);
}

void loop() {}
