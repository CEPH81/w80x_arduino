// Hace parpadear el LED con el temporizador 2 (sin delay en loop)
#include <HardwareTimer.h>

HardwareTimer MiTimer(2);          // temporizadores 0..5
volatile uint32_t ticks = 0;

void al_vencer() {                 // se llama desde la interrupcion: corta
  ticks++;
  digitalToggle(LED_BUILTIN);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  // cada 500 ms, periodico
  MiTimer.configure(500, TIM_UNIT_MS, TIM_AUTORELOAD_PRELOAD_ENABLE, al_vencer);
  MiTimer.start();
}

void loop() {
  Serial.printf("interrupciones: %u\n", ticks);
  delay(1000);
}
