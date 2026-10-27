/*
 * HardwareSerial.cpp - puertos serie del W80x sobre el driver UART del SDK.
 *
 * El driver guarda lo recibido en su propio buffer (interrupciones); aqui
 * se pasa, sin esperar, a un buffer circular para poder ofrecer
 * available() y peek().
 *
 * Parte de w80x_arduino. LGPL 2.1.
 */
#include "Arduino.h"
#include "HardwareSerial.h"
#include "wiring_private.h"

#include "wm_drv_uart.h"
#include "wm_dt_hw.h"
#include "wm_uart_reg_struct.h"

#define DRV_RX_BUF 512
#define DRV_TX_BUF 512

HardwareSerial Serial(0);
HardwareSerial Serial1(1);
HardwareSerial Serial2(2);
HardwareSerial Serial3(3);
HardwareSerial Serial4(4);
#if W80X_UART_COUNT > 5
HardwareSerial Serial5(5);
#endif

HardwareSerial::HardwareSerial(uint8_t port)
    : _port(port), _dev(nullptr), _baud(115200), _head(0), _tail(0)
{
}

void HardwareSerial::begin(unsigned long baud, uint32_t config)
{
    char name[6] = { 'u', 'a', 'r', 't', (char)('0' + _port), 0 };
    wm_device_t *dev = wm_dt_get_device_by_name(name);
    if (dev == NULL) {
        return;
    }
    if (dev->state != WM_DEV_ST_INITED) {
        dev = wm_drv_uart_init(name, DRV_RX_BUF, DRV_TX_BUF);
        if (dev == NULL) {
            return;
        }
    }
    wm_drv_uart_config_t cfg;
    cfg.baudrate  = (wm_uart_baudrate_t)baud;
    cfg.data_bits = (wm_uart_data_bits_t)(config & 0x03);
    cfg.parity    = (config & SERIAL_PARITY_E) ? WM_UART_PARITY_EVEN :
                    (config & SERIAL_PARITY_O) ? WM_UART_PARITY_ODD : WM_UART_PARITY_NONE;
    cfg.stop_bits = (config & SERIAL_STOP_2) ? WM_UART_STOP_BIT_2 : WM_UART_STOP_BIT_1;
    cfg.flow_ctrl = WM_UART_FLOW_CTRL_DISABLE;
    flush();
    wm_drv_uart_set_config(dev, &cfg);

    /* las patas del puerto quedan reservadas (digitalWrite no las toca) */
    wm_dt_hw_uart_t *hw = (wm_dt_hw_uart_t *)dev->hw;
    for (int i = 0; i < hw->pin_cfg_count; i++) {
        uint8_t p = hw->pin_cfg[i].pin_num;
        if (p < W80X_PIN_MUX_COUNT) {
            w80x_pin_state[p] = W80X_PIN_PERIPH;
        }
    }
    _baud = baud;
    _head = _tail = 0;
    _dev  = dev;
}

void HardwareSerial::end()
{
    if (_dev == nullptr) {
        return;
    }
    flush();
    if (_port != 0) { /* UART0 lo usa tambien printf() del sistema */
        wm_drv_uart_deinit(_dev);
        wm_dt_hw_uart_t *hw = (wm_dt_hw_uart_t *)_dev->hw;
        for (int i = 0; i < hw->pin_cfg_count; i++) {
            uint8_t p = hw->pin_cfg[i].pin_num;
            if (p < W80X_PIN_MUX_COUNT) {
                w80x_pin_state[p] = W80X_PIN_UNUSED;
            }
        }
    }
    _dev = nullptr;
}

void HardwareSerial::updateBaudRate(unsigned long baud)
{
    if (_dev) {
        flush();
        wm_drv_uart_set_baudrate(_dev, baud);
        _baud = baud;
    }
}

unsigned long HardwareSerial::baudRate()
{
    return _baud;
}

void HardwareSerial::fill()
{
    if (_dev == nullptr) {
        return;
    }
    for (;;) {
        uint16_t head = _head;
        uint16_t tail = _tail;
        uint16_t room;
        if (head >= tail) {
            room = SERIAL_RX_BUFFER_SIZE - head - (tail == 0 ? 1 : 0);
        } else {
            room = tail - head - 1;
        }
        if (room == 0) {
            return;
        }
        int n = wm_drv_uart_read(_dev, &_rx[head], room, 0);
        if (n <= 0) {
            return;
        }
        _head = (head + n) % SERIAL_RX_BUFFER_SIZE;
    }
}

int HardwareSerial::available()
{
    fill();
    return (SERIAL_RX_BUFFER_SIZE + _head - _tail) % SERIAL_RX_BUFFER_SIZE;
}

int HardwareSerial::peek()
{
    if (available() == 0) {
        return -1;
    }
    return _rx[_tail];
}

int HardwareSerial::read()
{
    if (available() == 0) {
        return -1;
    }
    uint8_t c = _rx[_tail];
    _tail     = (_tail + 1) % SERIAL_RX_BUFFER_SIZE;
    return c;
}

size_t HardwareSerial::readBytes(char *buffer, size_t length)
{
    size_t n = 0;
    while (n < length && available() > 0) {
        buffer[n++] = (char)read();
    }
    if (n < length && _dev) {
        /* lo que falta se espera en el driver, sin gastar CPU */
        int r = wm_drv_uart_read(_dev, (uint8_t *)buffer + n, length - n, _timeout);
        if (r > 0) {
            n += r;
        }
    }
    return n;
}

int HardwareSerial::availableForWrite()
{
    return _dev ? DRV_TX_BUF : 0;
}

void HardwareSerial::flush()
{
    if (_dev == nullptr) {
        return;
    }
    /* el FIFO debe quedar vacio durante 2 caracteres: entonces el buffer
       del driver tambien esta vacio (la interrupcion lo rellenaria) */
    wm_uart_reg_t *reg = (wm_uart_reg_t *)((wm_dt_hw_uart_t *)_dev->hw)->reg_base;
    uint32_t chr_us    = 11000000UL / (_baud ? _baud : 115200) + 1;
    uint32_t start     = millis();
    while (millis() - start < 2000) {
        if (reg->fifo_status.txfifo_count == 0) {
            delayMicroseconds(2 * chr_us);
            if (reg->fifo_status.txfifo_count == 0) {
                delayMicroseconds(chr_us); /* ultimo caracter en el registro de desplazamiento */
                return;
            }
        }
        yield();
    }
}

size_t HardwareSerial::write(uint8_t c)
{
    return write(&c, 1);
}

size_t HardwareSerial::write(const uint8_t *buffer, size_t size)
{
    if (_dev == nullptr || size == 0) {
        return 0;
    }
    int n = wm_drv_uart_write(_dev, buffer, size);
    return n > 0 ? (size_t)n : 0;
}

/* ---------- serialEvent ---------- */
extern "C" {
void serialEvent(void) __attribute__((weak));
void serialEvent1(void) __attribute__((weak));
void serialEvent2(void) __attribute__((weak));
void serialEvent3(void) __attribute__((weak));
void serialEvent4(void) __attribute__((weak));
void serialEvent5(void) __attribute__((weak));
}

void serialEventRun(void)
{
    if (serialEvent && Serial.available()) serialEvent();
    if (serialEvent1 && Serial1.available()) serialEvent1();
    if (serialEvent2 && Serial2.available()) serialEvent2();
    if (serialEvent3 && Serial3.available()) serialEvent3();
    if (serialEvent4 && Serial4.available()) serialEvent4();
#if W80X_UART_COUNT > 5
    if (serialEvent5 && Serial5.available()) serialEvent5();
#endif
}
