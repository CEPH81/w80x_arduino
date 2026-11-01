/*
 * LogTareas - log en color desde varias tareas de FreeRTOS a la vez.
 *
 * Tres tareas escriben al mismo tiempo y cada linea sale entera, con el nombre
 * de la tarea que la escribio:
 *   [1234][I][loopTask][MAIN] ...     el loop() de Arduino
 *   [1250][D][sensor][TEMP] ...       temperatura interna cada segundo (azul)
 *   [1300][W][vigia][HEAP] ...        avisa si baja la memoria libre (amarillo)
 *
 * Terminal con colores ANSI a 115200 baudios (PuTTY, Tera Term, VS Code).
 * Sirve para todas las placas W80x.
 */
#include <W80x_Log.h>

void tareaSensor(void *) {
  for (;;) {
    float t = temperatureRead();
    if (t > 70) LOGE("TEMP", "chip muy caliente: %.1f C", t);
    else        LOGD("TEMP", "chip a %.1f C", t);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void tareaVigia(void *) {
  uint32_t minimo = getFreeHeap();
  for (;;) {
    uint32_t libre = getFreeHeap();
    if (libre < minimo) {
      minimo = libre;
      LOGW("HEAP", "nuevo minimo de memoria libre: %u bytes", minimo);
    }
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void setup() {
  Log_Init(115200);
  Log_SetLevel(LOG_DEBUG);
  Log_SetTaskName(true);       // anade [tarea] a cada linea
  // Log_SetColors(false);     // para el Monitor serie del Arduino IDE
  delay(300);
  LOGI("MAIN", "arrancando tareas");
  xTaskCreate(tareaSensor, "sensor", 2048, NULL, 2, NULL);
  xTaskCreate(tareaVigia,  "vigia",  2048, NULL, 2, NULL);
}

void loop() {
  static uint32_t n = 0;
  LOGI("MAIN", "vuelta %lu, encendida hace %lu s", (unsigned long)n++, millis() / 1000);
  if (n % 5 == 0 && n <= 50) {                 // 10 bloques como maximo
    uint8_t *bloque = (uint8_t *)malloc(512);   // gasta memoria para que vigia avise
    LOGD("MAIN", "reservados 512 bytes en %p", bloque);
  }
  delay(1500);
}
