/*
  WMath.cpp - Part of the Wiring project - http://wiring.org.co
  Copyright (c) 2004-06 Hernando Barragan
  Modified 13 August 2006, David A. Mellis for Arduino - http://www.arduino.cc/
  W80x: random() usa el generador de numeros aleatorios por hardware si no
  se ha llamado a randomSeed().

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
*/
#include "Arduino.h"

static bool seeded = false;

void randomSeed(unsigned long seed)
{
    if (seed != 0) {
        srand(seed);
        seeded = true;
    }
}

static uint32_t rnd32()
{
    if (seeded) {
        return ((uint32_t)rand() << 16) ^ (uint32_t)rand();
    }
    return w80x_random();
}

long random(long howbig)
{
    if (howbig <= 0) {
        return 0;
    }
    return rnd32() % (uint32_t)howbig;
}

long random(long howsmall, long howbig)
{
    if (howsmall >= howbig) {
        return howsmall;
    }
    return random(howbig - howsmall) + howsmall;
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    if (in_max == in_min) {
        return out_min;
    }
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint16_t makeWord(uint16_t w) { return w; }
uint16_t makeWord(uint8_t h, uint8_t l) { return (h << 8) | l; }

unsigned long pulseIn(uint8_t pin, uint8_t state) { return pulseIn(pin, state, 1000000UL); }
