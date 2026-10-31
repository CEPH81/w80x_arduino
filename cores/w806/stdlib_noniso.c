#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "stdlib_noniso.h"

 

void reverse(char* begin, char* end) {
    char *is = begin;
    char *ie = end - 1;
    while(is < ie) {
        char tmp = *ie;
        *ie = *is;
        *is = tmp;
        ++is;
        --ie;
    }
}

char* ltoa(long value, char* result, int base) {
    if(base < 2 || base > 16) {
        *result = 0;
        return result;
    }

    char* out = result;
    long quotient = abs(value);

    do {
        const long tmp = quotient / base;
        *out = "0123456789abcdef"[quotient - (tmp * base)];
        ++out;
        quotient = tmp;
    } while(quotient);

    // Apply negative sign
    if(value < 0)
        *out++ = '-';

    reverse(result, out);
    *out = 0;
    return result;
}

char* itoa (int val, char *s, int radix) 
{
  return ltoa (val, s, radix);
}

char* ultoa(unsigned long value, char* result, int base) {
    if(base < 2 || base > 16) {
        *result = 0;
        return result;
    }

    char* out = result;
    unsigned long quotient = value;

    do {
        const unsigned long tmp = quotient / base;
        *out = "0123456789abcdef"[quotient - (tmp * base)];
        ++out;
        quotient = tmp;
    } while(quotient);

    reverse(result, out);
    *out = 0;
    return result;
}

char* utoa (unsigned int val, char *s, int radix)
{
  return ultoa (val, s, radix);
}

char * dtostrf(double number, signed char width, unsigned char prec, char *s) {
    bool negative = false;

    if (isnan(number)) {
        strcpy(s, "nan");
        return s;
    }
    if (isinf(number)) {
        strcpy(s, "inf");
        return s;
    }

    char* out = s;

    int fillme = width; // how many cells to fill for the integer part
    if (prec > 0) {
        fillme -= (prec+1);
    }

    // Handle negative numbers
    if (number < 0.0) {
        negative = true;
        fillme--;
        number = -number;
    }

    // Round correctly so that print(1.999, 2) prints as "2.00"
    // I optimized out most of the divisions
    double rounding = 2.0;
    for (uint8_t i = 0; i < prec; ++i)
        rounding *= 10.0;
    rounding = 1.0 / rounding;

    number += rounding;

    // Figure out how big our number really is
    double tenpow = 1.0;
    int digitcount = 1;
    while (number >= 10.0 * tenpow) {
        tenpow *= 10.0;
        digitcount++;
    }

    number /= tenpow;
    fillme -= digitcount;

    // Pad unused cells with spaces
    while (fillme-- > 0) {
        *out++ = ' ';
    }

    // Handle negative sign
    if (negative) *out++ = '-';

    // Print the digits, and if necessary, the decimal point
    digitcount += prec;
    int8_t digit = 0;
    while (digitcount-- > 0) {
        digit = (int8_t)number;
        if (digit > 9) digit = 9; // insurance
        *out++ = (char)('0' | digit);
        if ((digitcount == prec) && (prec > 0)) {
            *out++ = '.';
        }
        number -= digit;
        number *= 10.0;
    }

    // make sure the string is terminated
    *out = 0;
    return s;
}






 

/* ---- funciones de cadenas que faltan en minilibc ---- */
__attribute__((weak)) char *strndup(const char *s, size_t n)
{
    size_t l = 0;
    while (l < n && s[l]) l++;
    char *d = (char *)malloc(l + 1);
    if (d) {
        memcpy(d, s, l);
        d[l] = 0;
    }
    return d;
}

__attribute__((weak)) size_t strlcat(char *dest, const char *src, size_t len)
{
    size_t dl = 0, sl = strlen(src);
    while (dl < len && dest[dl]) dl++;
    if (dl == len) return len + sl;
    size_t c = sl < len - dl - 1 ? sl : len - dl - 1;
    memcpy(dest + dl, src, c);
    dest[dl + c] = 0;
    return dl + sl;
}

__attribute__((weak)) char *strtok_r(char *str, const char *delim, char **saveptr)
{
    char *s = str ? str : *saveptr;
    if (!s) return NULL;
    s += strspn(s, delim);
    if (!*s) {
        *saveptr = NULL;
        return NULL;
    }
    char *e = s + strcspn(s, delim);
    if (*e) {
        *e++ = 0;
        *saveptr = e;
    } else {
        *saveptr = NULL;
    }
    return s;
}

__attribute__((weak)) char *strsep(char **stringp, const char *delim)
{
    char *s = *stringp;
    if (!s) return NULL;
    char *e = s + strcspn(s, delim);
    if (*e) {
        *e++ = 0;
        *stringp = e;
    } else {
        *stringp = NULL;
    }
    return s;
}

__attribute__((weak)) void *memmem(const void *haystack, size_t hlen, const void *needle, size_t nlen)
{
    const unsigned char *h = (const unsigned char *)haystack;
    if (nlen == 0) return (void *)h;
    for (size_t i = 0; i + nlen <= hlen; i++) {
        if (h[i] == *(const unsigned char *)needle && memcmp(h + i, needle, nlen) == 0) return (void *)(h + i);
    }
    return NULL;
}
