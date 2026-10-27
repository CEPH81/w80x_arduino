/*
 * main.cpp - arranque del programa Arduino en W80x (WM IoT SDK).
 *
 * El SDK inicia el reloj, la memoria, la flash, NVS y UART0 y despues llama
 * a main() desde su tarea "main". Aqui se crea la tarea "loopTask", que
 * ejecuta setup() una vez y loop() sin fin.
 *
 * Parte de w80x_arduino. LGPL 2.1.
 */
#include "Arduino.h"
#include "wiring_private.h"
#include "wm_log.h"

TaskHandle_t loopTaskHandle = NULL;

/* los pide el SDK (los genera su sistema de compilacion) */
extern "C" {
const char *wm_sdk_build_time = __TIME__;
const char *wm_sdk_build_date = __DATE__;
}

extern "C" void initVariant(void) __attribute__((weak));
extern "C" void initVariant(void) {}

#ifndef ARDUINO_LOOP_PRIORITY
#define ARDUINO_LOOP_PRIORITY 1
#endif

#ifndef W80X_LOG_LEVEL
#define W80X_LOG_LEVEL WM_LOG_LEVEL_WARN
#endif

static void loopTask(void *arg)
{
    (void)arg;
    setup();
    for (;;) {
        loop();
        serialEventRun();
    }
}

extern "C" int main(void)
{
    wm_log_set_level((wm_log_level_t)W80X_LOG_LEVEL);
    w80x_init_digital();
    Serial.begin(115200);
    initVariant();
    xTaskCreate(loopTask, "loopTask", ARDUINO_LOOP_STACK_SIZE / sizeof(StackType_t), NULL,
                ARDUINO_LOOP_PRIORITY, &loopTaskHandle);
    return 0;
}

extern "C" void __cxa_pure_virtual(void)
{
    while (1) {
    }
}
