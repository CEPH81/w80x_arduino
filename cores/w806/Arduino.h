/*
 * Arduino.h - nucleo Arduino para W80x (W800, W801, W802, W806, Air101,
 * Air103) sobre WM IoT SDK v2.4.
 *
 * El programa corre en una tarea de FreeRTOS ("loopTask"); se pueden crear
 * otras tareas con xTaskCreate() y usar colas, semaforos, etc.
 *
 * Parte de w80x_arduino. LGPL 2.1.
 */
#ifndef Arduino_h
#define Arduino_h

#include "Common.h"
#include "pins_arduino.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
#include "WCharacter.h"
#include "WString.h"
#include "Stream.h"
#include "HardwareSerial.h"
#endif

#include "pgmspace.h"
#include "stdlib_noniso.h"

/* tamano de la pila de la tarea del programa (setup/loop), en bytes */
#ifndef ARDUINO_LOOP_STACK_SIZE
#define ARDUINO_LOOP_STACK_SIZE 8192
#endif

#endif /* Arduino_h */
