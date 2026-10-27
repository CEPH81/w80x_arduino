/*
 * SPI.h - bus SPI (maestro) del W80x, compatible con la libreria SPI de Arduino.
 *
 * Patas: SCK, MISO, MOSI y SS de la placa (pins_arduino.h / variant.h).
 * SS (CS) lo maneja el programa con digitalWrite, como en Arduino.
 * Reloj de 10 kHz a 20 MHz. Modos SPI_MODE0..3, MSBFIRST o LSBFIRST.
 *
 * Parte de w80x_arduino. LGPL 2.1.
 */
#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

#include <Arduino.h>

#define SPI_HAS_TRANSACTION 1

#define SPI_MODE0 0x00
#define SPI_MODE1 0x01
#define SPI_MODE2 0x02
#define SPI_MODE3 0x03

/* divisores de AVR (16 MHz): se convierten a frecuencia */
#define SPI_CLOCK_DIV2   2
#define SPI_CLOCK_DIV4   4
#define SPI_CLOCK_DIV8   8
#define SPI_CLOCK_DIV16  16
#define SPI_CLOCK_DIV32  32
#define SPI_CLOCK_DIV64  64
#define SPI_CLOCK_DIV128 128

class SPISettings
{
public:
    SPISettings() : _clock(1000000), _bitOrder(MSBFIRST), _dataMode(SPI_MODE0) {}
    SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode)
        : _clock(clock), _bitOrder(bitOrder), _dataMode(dataMode) {}
    uint32_t _clock;
    uint8_t _bitOrder;
    uint8_t _dataMode;
};

class SPIClass
{
public:
    SPIClass();
    void begin();
    void begin(int8_t sck, int8_t miso, int8_t mosi, int8_t ss = -1);
    void end();

    void beginTransaction(SPISettings settings);
    void endTransaction();

    uint8_t transfer(uint8_t data);
    uint16_t transfer16(uint16_t data);
    uint32_t transfer32(uint32_t data);
    void transfer(void *buf, size_t count);
    void transferBytes(const uint8_t *data, uint8_t *out, uint32_t size);
    void writeBytes(const uint8_t *data, uint32_t size);
    void write(uint8_t data) { transfer(data); }
    void write16(uint16_t data) { transfer16(data); }
    void write32(uint32_t data) { transfer32(data); }

    void setFrequency(uint32_t freq);
    void setClockDivider(uint8_t div) { setFrequency(16000000UL / (div ? div : 4)); }
    void setDataMode(uint8_t mode) { _settings._dataMode = mode & 3; }
    void setBitOrder(uint8_t order) { _settings._bitOrder = order; }
    void setHwCs(bool use) { (void)use; } /* CS siempre por programa */

    int8_t pinSS() { return _ss; }

    void usingInterrupt(int) {}
    void notUsingInterrupt(int) {}
    void attachInterrupt() {}
    void detachInterrupt() {}

private:
    bool xfer(const uint8_t *tx, uint8_t *rx, size_t len);
    int8_t _sck, _miso, _mosi, _ss;
    bool _started;
    bool _inTransaction;
    SPISettings _settings;
    void *_dev;
    SemaphoreHandle_t _lock;
};

extern SPIClass SPI;

#endif
