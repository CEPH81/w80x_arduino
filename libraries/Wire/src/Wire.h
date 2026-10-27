/*
 * Wire.h - bus I2C (maestro) del W80x, compatible con la libreria Wire de Arduino.
 *
 * Patas por defecto: SDA y SCL de pins_arduino.h (PA4 y PA1). Otras posibles:
 * Wire.begin(PB19, PB20) (las de Serial: no usar a la vez). Velocidad de
 * 1 kHz a 400 kHz (Wire.setClock). El W80x solo tiene modo maestro.
 *
 * Parte de w80x_arduino. LGPL 2.1.
 */
#ifndef TwoWire_h
#define TwoWire_h

#include <Arduino.h>

#ifndef I2C_BUFFER_LENGTH
#define I2C_BUFFER_LENGTH 128
#endif
#define BUFFER_LENGTH I2C_BUFFER_LENGTH

/* codigos de endTransmission() */
#define I2C_ERROR_OK          0
#define I2C_ERROR_LENGTH      1 /* demasiados datos */
#define I2C_ERROR_ADDR_NACK   2 /* nadie respondio a la direccion */
#define I2C_ERROR_DATA_NACK   3 /* el esclavo no acepto un dato */
#define I2C_ERROR_OTHER       4
#define I2C_ERROR_TIMEOUT     5

class TwoWire : public Stream
{
public:
    TwoWire();

    bool begin();
    bool begin(int sda, int scl, uint32_t frequency = 0);
    void begin(uint8_t address); /* esclavo: no disponible en W80x */
    void end();
    void setClock(uint32_t frequency);
    uint32_t getClock() { return _freq; }
    bool setPins(int sda, int scl);
    void setWireTimeout(uint32_t timeout_us = 25000, bool reset_with_timeout = false);
    void setTimeOut(uint16_t timeout_ms) { _timeout_us = (uint32_t)timeout_ms * 1000; }
    uint16_t getTimeOut() { return _timeout_us / 1000; }

    void beginTransmission(uint8_t address);
    void beginTransmission(int address) { beginTransmission((uint8_t)address); }
    uint8_t endTransmission(bool sendStop);
    uint8_t endTransmission() { return endTransmission(true); }

    size_t requestFrom(uint8_t address, size_t size, bool sendStop);
    uint8_t requestFrom(uint8_t address, uint8_t size, uint8_t sendStop) { return requestFrom(address, (size_t)size, (bool)sendStop); }
    uint8_t requestFrom(uint8_t address, uint8_t size) { return requestFrom(address, (size_t)size, true); }
    uint8_t requestFrom(int address, int size) { return requestFrom((uint8_t)address, (size_t)size, true); }
    uint8_t requestFrom(int address, int size, int sendStop) { return requestFrom((uint8_t)address, (size_t)size, (bool)sendStop); }

    size_t write(uint8_t data) override;
    size_t write(const uint8_t *data, size_t quantity) override;
    int available() override;
    int read() override;
    int peek() override;
    void flush() override;

    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    using Print::write;

    /* sin uso en W80x (solo maestro); se mantienen por compatibilidad */
    void onReceive(void (*)(int)) {}
    void onRequest(void (*)(void)) {}

private:
    bool init();
    bool waitXfer();
    bool sendByte(uint8_t b, uint32_t ctrl);
    bool readByte(uint8_t *b, uint32_t ctrl);
    void stop();

    int _sda, _scl;
    uint32_t _freq;
    uint32_t _timeout_us;
    bool _started;   /* begin() hecho */
    bool _busHeld;   /* ultima operacion sin STOP: la siguiente es inicio repetido */
    uint8_t _txAddr;
    bool _txActive;
    uint8_t _tx[I2C_BUFFER_LENGTH];
    size_t _txLen;
    uint8_t _rx[I2C_BUFFER_LENGTH];
    size_t _rxLen, _rxIdx;
    void *_reg;
};

extern TwoWire Wire;

#endif
