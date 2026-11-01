/*
 * W80x_Log.h - log por niveles con colores ANSI, estilo esp_log.
 * Misma interfaz que CH579_Log (nucleo de la CH579M): los programas pasan de una placa
 * a otra cambiando solo el #include.
 *
 *   Log_Init(115200);                 // Serial.begin(115200)
 *   Log_SetLevel(LOG_DEBUG);
 *   LOGI("MAIN", "valor=%d", x);      // [1234][I][MAIN] valor=5   (en verde)
 *
 * Formato: <color>[millis][nivel][tag] mensaje<reset>\r\n
 * Con Log_SetTaskName(true): [millis][nivel][tarea][tag] mensaje
 *
 * Colores: E rojo, W amarillo, I verde, D azul, V gris.
 * Se ven en terminales con ANSI (PuTTY, Tera Term, el Monitor serie de VS Code / PlatformIO).
 * El Monitor serie del Arduino IDE muestra los codigos de escape tal cual: alli use
 * Log_SetColors(false).
 *
 * Seguro entre tareas de FreeRTOS: cada linea sale entera, sin mezclarse con otra.
 * No llamar desde interrupciones.
 */
#ifndef W80X_LOG_H
#define W80X_LOG_H

#include <Arduino.h>

typedef enum {
    LOG_NONE = 0,
    LOG_ERROR,
    LOG_WARN,
    LOG_INFO,
    LOG_DEBUG,
    LOG_VERBOSE,
} LOG_LevelTypeDef;

#ifndef LOG_BUFFER_SIZE
#define LOG_BUFFER_SIZE 192
#endif

void Log_Init(unsigned long baud = 115200, Stream *out = nullptr);   /* por defecto, Serial */
void Log_SetLevel(LOG_LevelTypeDef level);
LOG_LevelTypeDef Log_GetLevel(void);
void Log_SetColors(bool enable);
void Log_SetTaskName(bool enable);     /* anade el nombre de la tarea de FreeRTOS */
void Log_Write(LOG_LevelTypeDef level, const char *tag, const char *fmt, ...)
    __attribute__((format(printf, 3, 4)));
void Log_Hex(LOG_LevelTypeDef level, const char *tag, const void *data, size_t len); /* volcado hexadecimal */

#define LOGE(tag, fmt, ...)  Log_Write(LOG_ERROR,   tag, fmt, ##__VA_ARGS__)
#define LOGW(tag, fmt, ...)  Log_Write(LOG_WARN,    tag, fmt, ##__VA_ARGS__)
#define LOGI(tag, fmt, ...)  Log_Write(LOG_INFO,    tag, fmt, ##__VA_ARGS__)
#define LOGD(tag, fmt, ...)  Log_Write(LOG_DEBUG,   tag, fmt, ##__VA_ARGS__)
#define LOGV(tag, fmt, ...)  Log_Write(LOG_VERBOSE, tag, fmt, ##__VA_ARGS__)

#endif
