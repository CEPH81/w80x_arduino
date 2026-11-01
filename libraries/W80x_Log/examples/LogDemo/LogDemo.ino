/*
 * LogDemo - log por niveles en color (el mismo ejemplo que en la CH579M).
 *
 * Cada 5 mensajes cambia de nivel/color y marca el inicio del bloque:
 *   verde (Info) -> amarillo (Warn) -> azul (Debug) -> rojo (Error) -> ...
 *
 * Use una terminal con colores ANSI (PuTTY, Tera Term, VS Code) a 115200 baudios.
 * En el Monitor serie del Arduino IDE, llame a Log_SetColors(false).
 * Sirve para todas las placas W80x (W800, W801, W802, W806, Air101, Air103).
 */
#include <W80x_Log.h>

static const LOG_LevelTypeDef ciclo[]  = { LOG_INFO, LOG_WARN, LOG_DEBUG, LOG_ERROR };
static const char *const      nombre[] = { "VERDE", "AMARILLO", "AZUL", "ROJO" };
const uint8_t NUM_COLORES = 4;
const uint8_t POR_BLOQUE  = 5;
uint32_t contador = 0;

void setup() {
  Log_Init(115200);
  Log_SetLevel(LOG_DEBUG);     // deja pasar los 4 niveles
  // Log_SetColors(false);     // para el Monitor serie del Arduino IDE
  delay(300);
  LOGI("MAIN", "W80x a %u MHz, memoria libre %u bytes", getCpuFrequencyMhz(), getFreeHeap());
}

void loop() {
  uint8_t idx = (uint8_t)((contador / POR_BLOQUE) % NUM_COLORES);
  LOG_LevelTypeDef nivel = ciclo[idx];

  if ((contador % POR_BLOQUE) == 0) {
    Log_Write(nivel, "CICLO", "--- inicio bloque %s ---", nombre[idx]);
  }
  Log_Write(nivel, "LOOP", "mensaje #%lu", (unsigned long)contador);
  contador++;
  delay(500);
}
