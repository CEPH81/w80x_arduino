/*
 * EEPROM.h - EEPROM emulada en la flash del W80x.
 *
 * Interfaz de la EEPROM de Arduino (Christopher Andrews / Paul Stoffregen):
 * read, write, update, get, put, length y EEPROM[i]. Tambien begin(tamano),
 * commit() y end() como en el ESP32 (aqui los cambios se guardan siempre al
 * momento; commit() no hace falta).
 *
 * Los datos van en la zona NVS de la flash (con nivelacion de desgaste), en
 * bloques de 64 bytes: escribir un byte solo reescribe su bloque. Sobreviven
 * a cargar otro programa por puerto serie.
 *
 * Parte de w80x_arduino. LGPL 2.1.
 */
#ifndef EEPROM_h
#define EEPROM_h

#include <Arduino.h>

#ifndef EEPROM_DEFAULT_SIZE
#define EEPROM_DEFAULT_SIZE 2048
#endif
#define EEPROM_MAX_SIZE 8192

class EEPROMClass;

struct EERef {
    EERef(EEPROMClass &e, int i) : ee(e), index(i) {}
    uint8_t operator*() const;
    operator uint8_t() const { return **this; }
    EERef &operator=(const EERef &ref) { return *this = *ref; }
    EERef &operator=(uint8_t in);
    EERef &operator+=(uint8_t in) { return *this = **this + in; }
    EERef &operator-=(uint8_t in) { return *this = **this - in; }
    EERef &operator*=(uint8_t in) { return *this = **this * in; }
    EERef &operator/=(uint8_t in) { return *this = **this / in; }
    EERef &operator^=(uint8_t in) { return *this = **this ^ in; }
    EERef &operator%=(uint8_t in) { return *this = **this % in; }
    EERef &operator&=(uint8_t in) { return *this = **this & in; }
    EERef &operator|=(uint8_t in) { return *this = **this | in; }
    EERef &operator<<=(uint8_t in) { return *this = **this << in; }
    EERef &operator>>=(uint8_t in) { return *this = **this >> in; }
    EERef &update(uint8_t in) { return in != *this ? *this = in : *this; }
    EERef &operator++() { return *this += 1; }
    EERef &operator--() { return *this -= 1; }
    uint8_t operator++(int) { uint8_t r = **this; ++(*this); return r; }
    uint8_t operator--(int) { uint8_t r = **this; --(*this); return r; }
    EEPROMClass &ee;
    int index;
};

class EEPROMClass
{
public:
    explicit EEPROMClass(uint16_t size = EEPROM_DEFAULT_SIZE);
    ~EEPROMClass();

    bool begin(size_t size = 0);   /* opcional: cambia el tamano */
    bool commit() { return flush(); }
    void end() { flush(); }

    uint8_t read(int idx);
    uint8_t write(int idx, uint8_t val); /* 1 si se guardo bien */
    uint8_t update(int idx, uint8_t val) { return write(idx, val); }
    uint16_t length() { return _size; }

    EERef operator[](int idx) { return EERef(*this, idx); }

    template <typename T> T &get(int idx, T &t)
    {
        readBlock(idx, (uint8_t *)&t, sizeof(T));
        return t;
    }
    template <typename T> const T &put(int idx, const T &t)
    {
        writeBlock(idx, (const uint8_t *)&t, sizeof(T));
        return t;
    }

    uint8_t readBlock(int idx, uint8_t *ptr, uint16_t size);
    uint8_t writeBlock(int idx, const uint8_t *ptr, uint16_t size);
    uint8_t eeprom_read_byte(int idx) { return read(idx); }
    uint8_t eeprom_write_byte(int idx, uint8_t val) { return write(idx, val); }

    /* cadenas (como la EEPROM del ESP32) */
    size_t writeString(int idx, const String &s);
    String readString(int idx);

    /* borra todo (0xFF) */
    bool clear();

private:
    bool load();
    bool flush();
    bool saveChunk(int c);
    uint16_t _size;
    uint8_t *_data;
    uint32_t *_dirty; /* un bit por bloque de 64 bytes */
};

inline uint8_t EERef::operator*() const { return ee.read(index); }
inline EERef &EERef::operator=(uint8_t in) { ee.write(index, in); return *this; }

extern EEPROMClass EEPROM;

#endif
