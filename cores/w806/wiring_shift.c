/*
 * wiring_shift.c - shiftIn/shiftOut y pulseIn para W80x.
 *
 * Parte de w80x_arduino. LGPL 2.1.
 */
#include "Arduino.h"

uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder)
{
    uint8_t value = 0;
    for (uint8_t i = 0; i < 8; ++i) {
        digitalWrite(clockPin, HIGH);
        if (bitOrder == LSBFIRST) {
            value |= digitalRead(dataPin) << i;
        } else {
            value |= digitalRead(dataPin) << (7 - i);
        }
        digitalWrite(clockPin, LOW);
    }
    return value;
}

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
    for (uint8_t i = 0; i < 8; i++) {
        if (bitOrder == LSBFIRST) {
            digitalWrite(dataPin, (val >> i) & 1);
        } else {
            digitalWrite(dataPin, (val >> (7 - i)) & 1);
        }
        digitalWrite(clockPin, HIGH);
        digitalWrite(clockPin, LOW);
    }
}

/* duracion (us) de un pulso de nivel "state"; 0 si se agota el tiempo */
unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout)
{
    uint32_t start = micros();
    state          = state ? HIGH : LOW;
    while (digitalRead(pin) == state) {
        if ((uint32_t)(micros() - start) > timeout) return 0;
    }
    while (digitalRead(pin) != state) {
        if ((uint32_t)(micros() - start) > timeout) return 0;
    }
    uint32_t begin = micros();
    while (digitalRead(pin) == state) {
        if ((uint32_t)(micros() - start) > timeout) return 0;
    }
    return micros() - begin;
}

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout)
{
    return pulseInLong(pin, state, timeout);
}
