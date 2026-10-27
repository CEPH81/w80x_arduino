/*
  Print.cpp - Base class that provides print() and println()
  Copyright (c) 2008 David A. Mellis.  All right reserved.
  Adaptado al nucleo W80x (WM IoT SDK): printf(), flush(), 64 bits.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "Arduino.h"
#include "Print.h"

size_t Print::write(const uint8_t *buffer, size_t size)
{
    size_t n = 0;
    while (size--) {
        if (write(*buffer++)) {
            n++;
        } else {
            break;
        }
    }
    return n;
}

size_t Print::vprintf(const char *format, va_list arg)
{
    char loc[64];
    char *buf = loc;
    va_list copy;
    va_copy(copy, arg);
    int len = vsnprintf(loc, sizeof(loc), format, copy);
    va_end(copy);
    if (len < 0) {
        return 0;
    }
    if (len >= (int)sizeof(loc)) {
        buf = (char *)malloc(len + 1);
        if (buf == NULL) {
            return 0;
        }
        len = vsnprintf(buf, len + 1, format, arg);
    }
    size_t n = write((const uint8_t *)buf, len);
    if (buf != loc) {
        free(buf);
    }
    return n;
}

size_t Print::printf(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    size_t n = vprintf(format, arg);
    va_end(arg);
    return n;
}

size_t Print::print(const __FlashStringHelper *ifsh) { return print(reinterpret_cast<const char *>(ifsh)); }
size_t Print::print(const String &s) { return write(s.c_str(), s.length()); }
size_t Print::print(const char str[]) { return write(str); }
size_t Print::print(char c) { return write(c); }
size_t Print::print(unsigned char b, int base) { return print((unsigned long)b, base); }
size_t Print::print(int n, int base) { return print((long)n, base); }
size_t Print::print(unsigned int n, int base) { return print((unsigned long)n, base); }

size_t Print::print(long n, int base)
{
    if (base == 0) {
        return write(n);
    } else if (base == 10) {
        if (n < 0) {
            int t = print('-');
            return printNumber(-(unsigned long)n, 10) + t;
        }
        return printNumber(n, 10);
    }
    return printNumber(n, base);
}

size_t Print::print(unsigned long n, int base)
{
    if (base == 0) {
        return write(n);
    }
    return printNumber(n, base);
}

size_t Print::print(long long n, int base)
{
    if (base == 0) {
        return write(n);
    } else if (base == 10) {
        if (n < 0) {
            int t = print('-');
            return printULLNumber(-(unsigned long long)n, 10) + t;
        }
        return printULLNumber(n, 10);
    }
    return printULLNumber(n, base);
}

size_t Print::print(unsigned long long n, int base)
{
    if (base == 0) {
        return write(n);
    }
    return printULLNumber(n, base);
}

size_t Print::print(double n, int digits) { return printFloat(n, digits); }
size_t Print::print(const Printable &x) { return x.printTo(*this); }

size_t Print::println(void) { return write("\r\n"); }

#define PRINTLN_IMPL(...)          \
    size_t n = print(__VA_ARGS__); \
    n += println();                \
    return n;

size_t Print::println(const __FlashStringHelper *ifsh) { PRINTLN_IMPL(ifsh) }
size_t Print::println(const String &s) { PRINTLN_IMPL(s) }
size_t Print::println(const char c[]) { PRINTLN_IMPL(c) }
size_t Print::println(char c) { PRINTLN_IMPL(c) }
size_t Print::println(unsigned char b, int base) { PRINTLN_IMPL(b, base) }
size_t Print::println(int num, int base) { PRINTLN_IMPL(num, base) }
size_t Print::println(unsigned int num, int base) { PRINTLN_IMPL(num, base) }
size_t Print::println(long num, int base) { PRINTLN_IMPL(num, base) }
size_t Print::println(unsigned long num, int base) { PRINTLN_IMPL(num, base) }
size_t Print::println(long long num, int base) { PRINTLN_IMPL(num, base) }
size_t Print::println(unsigned long long num, int base) { PRINTLN_IMPL(num, base) }
size_t Print::println(double num, int digits) { PRINTLN_IMPL(num, digits) }
size_t Print::println(const Printable &x) { PRINTLN_IMPL(x) }

size_t Print::printNumber(unsigned long n, uint8_t base)
{
    char buf[8 * sizeof(long) + 1];
    char *str = &buf[sizeof(buf) - 1];

    *str = '\0';
    if (base < 2) {
        base = 10;
    }
    do {
        char c = n % base;
        n /= base;
        *--str = c < 10 ? c + '0' : c + 'A' - 10;
    } while (n);

    return write(str);
}

size_t Print::printULLNumber(unsigned long long n, uint8_t base)
{
    char buf[8 * sizeof(long long) + 1];
    char *str = &buf[sizeof(buf) - 1];

    *str = '\0';
    if (base < 2) {
        base = 10;
    }
    do {
        char c = n % base;
        n /= base;
        *--str = c < 10 ? c + '0' : c + 'A' - 10;
    } while (n);

    return write(str);
}

size_t Print::printFloat(double number, uint8_t digits)
{
    size_t n = 0;

    if (isnan(number)) return print("nan");
    if (isinf(number)) return print("inf");
    if (number > 4294967040.0) return print("ovf");
    if (number < -4294967040.0) return print("-ovf");

    if (number < 0.0) {
        n += print('-');
        number = -number;
    }

    double rounding = 0.5;
    for (uint8_t i = 0; i < digits; ++i) {
        rounding /= 10.0;
    }
    number += rounding;

    unsigned long int_part = (unsigned long)number;
    double remainder = number - (double)int_part;
    n += print(int_part);

    if (digits > 0) {
        n += print('.');
    }
    while (digits-- > 0) {
        remainder *= 10.0;
        unsigned int toPrint = (unsigned int)remainder;
        n += print(toPrint);
        remainder -= toPrint;
    }
    return n;
}
