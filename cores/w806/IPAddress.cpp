/*
 * IPAddress.cpp - direccion IPv4.
 * Parte de w80x_arduino. LGPL 2.1.
 */
#include "Arduino.h"
#include "IPAddress.h"
#include "Print.h"

const IPAddress INADDR_NONE(0, 0, 0, 0);

bool IPAddress::fromString(const char *address)
{
    uint16_t acc = 0;
    uint8_t dots = 0;
    bool digit   = false;
    while (*address) {
        char c = *address++;
        if (c >= '0' && c <= '9') {
            acc   = acc * 10 + (c - '0');
            digit = true;
            if (acc > 255) return false;
        } else if (c == '.') {
            if (dots == 3 || !digit) return false;
            _addr.bytes[dots++] = acc;
            acc   = 0;
            digit = false;
        } else {
            return false;
        }
    }
    if (dots != 3 || !digit) return false;
    _addr.bytes[3] = acc;
    return true;
}

size_t IPAddress::printTo(Print &p) const
{
    size_t n = 0;
    for (int i = 0; i < 3; i++) {
        n += p.print(_addr.bytes[i], DEC);
        n += p.print('.');
    }
    n += p.print(_addr.bytes[3], DEC);
    return n;
}

String IPAddress::toString() const
{
    char s[16];
    snprintf(s, sizeof(s), "%u.%u.%u.%u", _addr.bytes[0], _addr.bytes[1], _addr.bytes[2], _addr.bytes[3]);
    return String(s);
}
