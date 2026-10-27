/*
 * HardwareSerial.h - puertos serie del W80x sobre el driver UART del SDK.
 *
 * Serial  = UART0 (PB19 TX, PB20 RX): el del cargador USB-serie.
 * Serial1..Serial5 = UART1..UART5 (Serial5 no existe en W800/Air101).
 * Patas de cada puerto: ver pins_arduino.h de la placa.
 *
 * Parte de w80x_arduino. LGPL 2.1.
 */
#ifndef HardwareSerial_h
#define HardwareSerial_h

#include <stdint.h>
#include "Stream.h"

/* configuracion: bits 0-1 datos (5..8), bits 2-3 paridad, bit 4 parada */
#define SERIAL_DATA_5   0x00
#define SERIAL_DATA_6   0x01
#define SERIAL_DATA_7   0x02
#define SERIAL_DATA_8   0x03
#define SERIAL_PARITY_N 0x00
#define SERIAL_PARITY_E 0x04
#define SERIAL_PARITY_O 0x08
#define SERIAL_STOP_1   0x00
#define SERIAL_STOP_2   0x10

#define SERIAL_5N1 (SERIAL_DATA_5 | SERIAL_PARITY_N | SERIAL_STOP_1)
#define SERIAL_6N1 (SERIAL_DATA_6 | SERIAL_PARITY_N | SERIAL_STOP_1)
#define SERIAL_7N1 (SERIAL_DATA_7 | SERIAL_PARITY_N | SERIAL_STOP_1)
#define SERIAL_8N1 (SERIAL_DATA_8 | SERIAL_PARITY_N | SERIAL_STOP_1)
#define SERIAL_5N2 (SERIAL_DATA_5 | SERIAL_PARITY_N | SERIAL_STOP_2)
#define SERIAL_6N2 (SERIAL_DATA_6 | SERIAL_PARITY_N | SERIAL_STOP_2)
#define SERIAL_7N2 (SERIAL_DATA_7 | SERIAL_PARITY_N | SERIAL_STOP_2)
#define SERIAL_8N2 (SERIAL_DATA_8 | SERIAL_PARITY_N | SERIAL_STOP_2)
#define SERIAL_5E1 (SERIAL_DATA_5 | SERIAL_PARITY_E | SERIAL_STOP_1)
#define SERIAL_6E1 (SERIAL_DATA_6 | SERIAL_PARITY_E | SERIAL_STOP_1)
#define SERIAL_7E1 (SERIAL_DATA_7 | SERIAL_PARITY_E | SERIAL_STOP_1)
#define SERIAL_8E1 (SERIAL_DATA_8 | SERIAL_PARITY_E | SERIAL_STOP_1)
#define SERIAL_5E2 (SERIAL_DATA_5 | SERIAL_PARITY_E | SERIAL_STOP_2)
#define SERIAL_6E2 (SERIAL_DATA_6 | SERIAL_PARITY_E | SERIAL_STOP_2)
#define SERIAL_7E2 (SERIAL_DATA_7 | SERIAL_PARITY_E | SERIAL_STOP_2)
#define SERIAL_8E2 (SERIAL_DATA_8 | SERIAL_PARITY_E | SERIAL_STOP_2)
#define SERIAL_5O1 (SERIAL_DATA_5 | SERIAL_PARITY_O | SERIAL_STOP_1)
#define SERIAL_6O1 (SERIAL_DATA_6 | SERIAL_PARITY_O | SERIAL_STOP_1)
#define SERIAL_7O1 (SERIAL_DATA_7 | SERIAL_PARITY_O | SERIAL_STOP_1)
#define SERIAL_8O1 (SERIAL_DATA_8 | SERIAL_PARITY_O | SERIAL_STOP_1)
#define SERIAL_5O2 (SERIAL_DATA_5 | SERIAL_PARITY_O | SERIAL_STOP_2)
#define SERIAL_6O2 (SERIAL_DATA_6 | SERIAL_PARITY_O | SERIAL_STOP_2)
#define SERIAL_7O2 (SERIAL_DATA_7 | SERIAL_PARITY_O | SERIAL_STOP_2)
#define SERIAL_8O2 (SERIAL_DATA_8 | SERIAL_PARITY_O | SERIAL_STOP_2)

#ifndef SERIAL_RX_BUFFER_SIZE
#define SERIAL_RX_BUFFER_SIZE 256
#endif

struct device_structure;

class HardwareSerial : public Stream
{
public:
    explicit HardwareSerial(uint8_t port);

    /* rxPin/txPin: otras patas para el puerto (ver pins_mux.h), -1 = por defecto */
    void begin(unsigned long baud = 115200, uint32_t config = SERIAL_8N1, int8_t rxPin = -1, int8_t txPin = -1);
    void end();
    void updateBaudRate(unsigned long baud);
    unsigned long baudRate();

    int available() override;
    int peek() override;
    int read() override;
    size_t readBytes(char *buffer, size_t length) override;
    size_t readBytes(uint8_t *buffer, size_t length) override { return readBytes((char *)buffer, length); }
    int availableForWrite() override;
    void flush() override;
    size_t write(uint8_t c) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    using Print::write;

    operator bool() const { return _dev != nullptr; }
    uint8_t port() const { return _port; }

private:
    void fill();
    uint8_t _port;
    struct device_structure *_dev;
    unsigned long _baud;
    uint16_t _head, _tail;
    uint8_t _rx[SERIAL_RX_BUFFER_SIZE];
};

extern HardwareSerial Serial;
extern HardwareSerial Serial1;
extern HardwareSerial Serial2;
extern HardwareSerial Serial3;
extern HardwareSerial Serial4;
#if W80X_UART_COUNT > 5
extern HardwareSerial Serial5;
#endif

/* llamada despues de cada loop() si el sketch define serialEvent() */
void serialEventRun(void);

#endif
