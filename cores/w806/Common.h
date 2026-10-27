/*
 * Common.h - tipos, constantes y funciones basicas de Arduino para W80x
 * (nucleo sobre WM IoT SDK).
 *
 * Parte de w80x_arduino. LGPL 2.1.
 */
#ifndef Common_h
#define Common_h

#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "Binary.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t byte;
typedef bool boolean;
typedef uint16_t word;

/* ---------- niveles y modos de las patas ---------- */
#define HIGH 0x1
#define LOW  0x0

#define INPUT          0x01
#define OUTPUT         0x02
#define INPUT_PULLUP   0x04
#define INPUT_PULLDOWN 0x08
/* compatibilidad con el nucleo anterior: pinMode(pin, ANALOG_INPUT/PWM_OUT) */
#define ANALOG_INPUT   0x10
#define PWM_OUT        0x20

/* modos de attachInterrupt() (mismos valores que wm_gpio_intr_mode_t) */
#define FALLING    1
#define RISING     2
#define CHANGE     3
#define ONLOW      4
#define ONHIGH     5
#define LOW_LEVEL  ONLOW   /* nombres del nucleo anterior */
#define HIGH_LEVEL ONHIGH

typedef enum {
    LSBFIRST = 0,
    MSBFIRST = 1,
} BitOrder;

/* ---------- matematicas ---------- */
#ifdef PI
#undef PI
#endif
#ifdef HALF_PI
#undef HALF_PI
#endif
#ifdef TWO_PI
#undef TWO_PI
#endif
#define PI         3.1415926535897932384626433832795
#define HALF_PI    1.5707963267948966192313216916398
#define TWO_PI     6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105
#define EULER      2.718281828459045235360287471352

#define SERIAL  0x0
#define DISPLAY 0x1

#ifndef __cplusplus
/* en C++ son plantillas (ver mas abajo), asi no chocan con <algorithm> */
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef abs
#define abs(x) ((x) > 0 ? (x) : -(x))
#endif
#endif

#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
#define radians(deg) ((deg) * DEG_TO_RAD)
#define degrees(rad) ((rad) * RAD_TO_DEG)
#define sq(x) ((x) * (x))

/* ---------- bits ---------- */
#define lowByte(w)  ((uint8_t)((w) & 0xff))
#define highByte(w) ((uint8_t)((w) >> 8))
#define bitRead(value, bit)  (((value) >> (bit)) & 0x01)
#define bitSet(value, bit)   ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitToggle(value, bit) ((value) ^= (1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))
#define bit(b) (1UL << (b))
#define maskSet(value, mask)   ((value) |= (mask))
#define maskClear(value, mask) ((value) &= ~(mask))

/* ---------- tiempos ---------- */
#define clockCyclesPerMicrosecond()  (F_CPU / 1000000L)
#define clockCyclesToMicroseconds(a) ((a) / clockCyclesPerMicrosecond())
#define microsecondsToClockCycles(a) ((a) * clockCyclesPerMicrosecond())

/* ---------- interrupciones ---------- */
void w80x_interrupts(void);
void w80x_no_interrupts(void);
#define interrupts()   w80x_interrupts()
#define noInterrupts() w80x_no_interrupts()

/* ---------- acceso directo a los puertos (estilo AVR) ---------- */
#define W80X_GPIOA_BASE 0x40011200UL
#define W80X_GPIOB_BASE 0x40011400UL
#define digitalPinToPort(pin)      ((pin) < 16 ? 0 : 1)
#define digitalPinToBitMask(pin)   (1UL << ((pin) < 16 ? (pin) : (pin) - 16))
#define portOutputRegister(port)   ((volatile uint32_t *)((port) ? W80X_GPIOB_BASE : W80X_GPIOA_BASE))
#define portInputRegister(port)    ((volatile uint32_t *)((port) ? W80X_GPIOB_BASE : W80X_GPIOA_BASE))
#define portModeRegister(port)     ((volatile uint32_t *)(((port) ? W80X_GPIOB_BASE : W80X_GPIOA_BASE) + 0x08))
#define digitalPinToTimer(pin)     (0)
#define digitalPinToInterrupt(pin) (pin)
#define digitalPinHasPWM(pin)      (w80x_pin_pwm_channel(pin) >= 0)
#define analogInputToDigitalPin(p) (p)
#define NOT_A_PIN                  (-1)
#define NOT_AN_INTERRUPT           (-1)

/* ---------- E/S digital ---------- */
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);
void digitalToggle(uint8_t pin);

/* ---------- E/S analogica ---------- */
int analogRead(uint8_t pin);                 /* 0..(2^bits - 1), 12 bits por defecto */
uint32_t analogReadMilliVolts(uint8_t pin);  /* tension en mV (calibrada de fabrica) */
void analogReadResolution(uint8_t bits);
void analogReference(uint8_t mode);          /* sin efecto: referencia interna fija */
void analogWrite(uint8_t pin, int value);    /* PWM, 0..255 por defecto */
void analogWriteResolution(uint8_t bits);    /* 1..8 bits (el PWM del W80x es de 8 bits) */
void analogWriteFrequency(uint32_t freq);    /* Hz, por defecto 1000 */
int w80x_pin_pwm_channel(uint8_t pin);       /* -1 si la pata no tiene PWM */
int w80x_pin_adc_channel(uint8_t pin);       /* -1 si la pata no es analogica */
float temperatureRead(void);                 /* sensor interno, grados C */

/* ---------- tiempo ---------- */
uint32_t millis(void);
uint32_t micros(void);
void delay(uint32_t ms);
void delayMicroseconds(uint32_t us);
void yield(void);

/* ---------- interrupciones externas ---------- */
typedef void (*voidFuncPtr)(void);
typedef void (*voidFuncPtrArg)(void *);
void attachInterrupt(uint8_t pin, voidFuncPtr callback, int mode);
void attachInterruptArg(uint8_t pin, voidFuncPtrArg callback, void *arg, int mode);
void detachInterrupt(uint8_t pin);

/* ---------- varios ---------- */
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout);
unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout);

uint32_t getCpuFrequencyMhz(void);
uint32_t getFreeHeap(void);
void w80x_restart(void);

/* numeros aleatorios del generador por hardware */
uint32_t w80x_random(void);

void setup(void);
void loop(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
#include <algorithm>
#include <cmath>
#include <cstdlib>

template <class T, class L>
inline auto min(const T &a, const L &b) -> decltype((b < a) ? b : a) { return (b < a) ? b : a; }
template <class T, class L>
inline auto max(const T &a, const L &b) -> decltype((b < a) ? b : a) { return (a < b) ? b : a; }
using std::abs;
using ::round;

#define word(...) makeWord(__VA_ARGS__)
uint16_t makeWord(uint16_t w);
uint16_t makeWord(uint8_t h, uint8_t l);

long random(long);
long random(long, long);
void randomSeed(unsigned long);
long map(long, long, long, long, long);

unsigned long pulseIn(uint8_t pin, uint8_t state);
#endif

#endif /* Common_h */
