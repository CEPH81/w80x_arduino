/*
 * W80x_Log.cpp - ver W80x_Log.h. Parte de w80x_arduino. LGPL 2.1.
 */
#include "W80x_Log.h"
#include <stdarg.h>
#include <stdio.h>

static LOG_LevelTypeDef  s_level  = LOG_INFO;
static Stream           *s_out    = nullptr;
static bool              s_colors = true;
static bool              s_task   = false;
static SemaphoreHandle_t s_mutex  = nullptr;

static const char *const s_tag[]   = { "", "E", "W", "I", "D", "V" };
static const char *const s_color[] = { "", "\x1b[31m", "\x1b[33m", "\x1b[32m", "\x1b[34m", "\x1b[90m" };
#define COLOR_RESET "\x1b[0m"

void Log_Init(unsigned long baud, Stream *out)
{
    if (!s_mutex) s_mutex = xSemaphoreCreateMutex();
    if (out) {
        s_out = out;
    } else {
        Serial.begin(baud);
        s_out = &Serial;
    }
}

void Log_SetLevel(LOG_LevelTypeDef level) { s_level = level; }
LOG_LevelTypeDef Log_GetLevel(void)       { return s_level; }
void Log_SetColors(bool enable)           { s_colors = enable; }
void Log_SetTaskName(bool enable)         { s_task = enable; }

static int cabecera(char *buf, size_t n, LOG_LevelTypeDef level, const char *tag)
{
    if (s_task)
        return snprintf(buf, n, "%s[%lu][%s][%s][%s] ", s_colors ? s_color[level] : "",
                        (unsigned long)millis(), s_tag[level], pcTaskGetName(NULL), tag);
    return snprintf(buf, n, "%s[%lu][%s][%s] ", s_colors ? s_color[level] : "",
                    (unsigned long)millis(), s_tag[level], tag);
}

static void emitir(const char *buf, int len)
{
    if (s_mutex) xSemaphoreTake(s_mutex, portMAX_DELAY);
    s_out->write((const uint8_t *)buf, (size_t)len);
    if (s_colors) s_out->print(COLOR_RESET);
    s_out->print("\r\n");
    if (s_mutex) xSemaphoreGive(s_mutex);
}

void Log_Write(LOG_LevelTypeDef level, const char *tag, const char *fmt, ...)
{
    if (!s_out || level == LOG_NONE || level > s_level) return;

    char buf[LOG_BUFFER_SIZE];
    int len = cabecera(buf, sizeof(buf), level, tag);
    if (len < 0) return;
    if (len < (int)sizeof(buf)) {
        va_list args;
        va_start(args, fmt);
        int n = vsnprintf(buf + len, sizeof(buf) - (size_t)len, fmt, args);
        va_end(args);
        if (n > 0) len += n;
    }
    if (len > (int)sizeof(buf) - 1) len = (int)sizeof(buf) - 1;
    emitir(buf, len);
}

void Log_Hex(LOG_LevelTypeDef level, const char *tag, const void *data, size_t len)
{
    if (!s_out || level == LOG_NONE || level > s_level) return;
    const uint8_t *p = (const uint8_t *)data;
    for (size_t i = 0; i < len; i += 16) {
        char buf[LOG_BUFFER_SIZE];
        int n = cabecera(buf, sizeof(buf), level, tag);
        n += snprintf(buf + n, sizeof(buf) - n, "%04x:", (unsigned)i);
        for (size_t j = i; j < i + 16 && j < len && n < (int)sizeof(buf) - 4; j++)
            n += snprintf(buf + n, sizeof(buf) - n, " %02x", p[j]);
        emitir(buf, n);
    }
}
