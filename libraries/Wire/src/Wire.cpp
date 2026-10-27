/*
 * Wire.cpp - bus I2C (maestro) del W80x.
 *
 * El driver del SDK prepara el periferico (reloj y patas); las transferencias
 * se hacen aqui sobre los registros, con tiempo de espera en cada paso, para
 * poder ofrecer lo que pide Wire: transmisiones de 0 bytes (escaner),
 * inicio repetido (endTransmission(false) + requestFrom) y codigos de error.
 *
 * Parte de w80x_arduino. LGPL 2.1.
 */
#include "Wire.h"
#include "wiring_private.h"

extern "C" {
#include "wm_drv_i2c.h"
#include "wm_dt_hw.h"
#include "wm_i2c_reg_struct.h"
#include "wm_ll_i2c.h"
}

#define I2C_REF_CLOCK 40000000UL /* reloj APB */
#define REG ((wm_i2c_reg_t *)_reg)

TwoWire Wire;

TwoWire::TwoWire()
    : _sda(PIN_WIRE_SDA), _scl(PIN_WIRE_SCL), _freq(100000), _timeout_us(25000), _started(false),
      _busHeld(false), _txAddr(0), _txActive(false), _txLen(0), _rxLen(0), _rxIdx(0), _reg(nullptr)
{
}

bool TwoWire::setPins(int sda, int scl)
{
    if (_started) {
        return false; /* llamar antes de begin() */
    }
    if (sda >= 0) _sda = sda;
    if (scl >= 0) _scl = scl;
    return true;
}

bool TwoWire::init()
{
    wm_device_t *dev = wm_dt_get_device_by_name("i2c");
    if (dev == nullptr) {
        return false;
    }
    if (dev->state != WM_DEV_ST_INITED && wm_drv_i2c_init("i2c") == nullptr) {
        return false;
    }
    wm_dt_hw_i2c_t *hw = (wm_dt_hw_i2c_t *)dev->hw;
    /* patas: si no son las del arbol de dispositivos, se cambian */
    for (int i = 0; i < hw->pin_cfg_count; i++) {
        uint8_t p = hw->pin_cfg[i].pin_num;
        if (p != _sda && p != _scl) {
            w80x_pin_release(p);
        } else if (p < W80X_PIN_MUX_COUNT) {
            w80x_pin_state[p] = W80X_PIN_PERIPH;
        }
    }
    if (!w80x_pin_attach_func(_sda, W80X_F_I2C_SDA) || !w80x_pin_attach_func(_scl, W80X_F_I2C_SCL)) {
        return false;
    }
    _reg = (void *)hw->reg_base;
    return true;
}

bool TwoWire::begin()
{
    if (!_started) {
        if (!init()) {
            return false;
        }
        _started = true;
    }
    setClock(_freq);
    _busHeld = false;
    return true;
}

bool TwoWire::begin(int sda, int scl, uint32_t frequency)
{
    if (_started && (sda != _sda || scl != _scl)) {
        end();
    }
    setPins(sda, scl);
    if (frequency) {
        _freq = frequency;
    }
    return begin();
}

void TwoWire::begin(uint8_t address)
{
    (void)address; /* el controlador I2C del W80x no tiene modo esclavo */
    begin();
}

void TwoWire::end()
{
    if (!_started) {
        return;
    }
    wm_device_t *dev = wm_dt_get_device_by_name("i2c");
    if (dev) {
        wm_drv_i2c_deinit(dev);
    }
    w80x_pin_release(_sda);
    w80x_pin_release(_scl);
    _started = false;
    _reg     = nullptr;
}

void TwoWire::setClock(uint32_t frequency)
{
    if (frequency < 1000) frequency = 1000;
    if (frequency > 400000) frequency = 400000;
    _freq = frequency;
    if (_reg) {
        /* como el SDK: cada ciclo de reloj suma medio periodo de espera (5,5 en vez de 5) */
        uint32_t div = I2C_REF_CLOCK * 2 / (11 * frequency) - 1;
        wm_ll_i2c_set_clock_div_low(REG, div & 0xff);
        wm_ll_i2c_set_clock_div_high(REG, (div >> 8) & 0xff);
        wm_ll_i2c_set_enable(REG, 0);
        wm_ll_i2c_set_enable(REG, 1);
        wm_ll_i2c_disable_int(REG);
    }
}

void TwoWire::setWireTimeout(uint32_t timeout_us, bool reset_with_timeout)
{
    (void)reset_with_timeout;
    _timeout_us = timeout_us ? timeout_us : 25000;
}

bool TwoWire::waitXfer()
{
    uint32_t t0 = micros();
    while (wm_ll_i2c_is_in_xfer(REG)) {
        if ((uint32_t)(micros() - t0) > _timeout_us) {
            return false;
        }
    }
    return true;
}

/* envia un byte; false si no hubo ACK (o se agoto el tiempo: _rxIdx = 0xFFFF) */
bool TwoWire::sendByte(uint8_t b, uint32_t ctrl)
{
    wm_ll_i2c_send_one_byte(REG, b);
    wm_ll_i2c_set_ctrl(REG, ctrl);
    if (!waitXfer()) {
        _rxIdx = 0xFFFF;
        return false;
    }
    /* el SDK espera un poco a que el esclavo de el ACK */
    for (int i = 0; i < 1024; i++) {
        if (!wm_ll_i2c_get_state_rx_ack(REG)) {
            return true;
        }
    }
    return false;
}

bool TwoWire::readByte(uint8_t *b, uint32_t ctrl)
{
    wm_ll_i2c_set_ctrl(REG, ctrl);
    if (!waitXfer()) {
        return false;
    }
    *b = wm_ll_i2c_recieve_one_byte(REG);
    return true;
}

void TwoWire::stop()
{
    wm_ll_i2c_set_ctrl(REG, WM_LL_I2C_CR_STO);
    waitXfer();
    _busHeld = false;
}

void TwoWire::beginTransmission(uint8_t address)
{
    _txAddr   = address;
    _txLen    = 0;
    _txActive = true;
}

uint8_t TwoWire::endTransmission(bool sendStop)
{
    if (!_started && !begin()) {
        return I2C_ERROR_OTHER;
    }
    if (!_txActive) {
        return I2C_ERROR_OTHER;
    }
    _txActive = false;
    _rxIdx    = 0;
    if (!sendByte((uint8_t)(_txAddr << 1), WM_LL_I2C_CR_STA | WM_LL_I2C_CR_WR)) {
        uint8_t err = (_rxIdx == 0xFFFF) ? I2C_ERROR_TIMEOUT : I2C_ERROR_ADDR_NACK;
        stop();
        _rxIdx = 0;
        return err;
    }
    for (size_t i = 0; i < _txLen; i++) {
        if (!sendByte(_tx[i], WM_LL_I2C_CR_WR)) {
            uint8_t err = (_rxIdx == 0xFFFF) ? I2C_ERROR_TIMEOUT : I2C_ERROR_DATA_NACK;
            stop();
            _rxIdx = 0;
            return err;
        }
    }
    if (sendStop) {
        stop();
    } else {
        _busHeld = true;
    }
    return I2C_ERROR_OK;
}

size_t TwoWire::requestFrom(uint8_t address, size_t size, bool sendStop)
{
    _rxLen = _rxIdx = 0;
    if (!_started && !begin()) {
        return 0;
    }
    if (size == 0) {
        return 0;
    }
    if (size > I2C_BUFFER_LENGTH) {
        size = I2C_BUFFER_LENGTH;
    }
    _rxIdx = 0;
    if (!sendByte((uint8_t)((address << 1) | 1), WM_LL_I2C_CR_STA | WM_LL_I2C_CR_WR)) {
        stop();
        _rxIdx = 0;
        return 0;
    }
    for (size_t i = 0; i < size; i++) {
        uint32_t ctrl = WM_LL_I2C_CR_RD;
        if (i == size - 1) {
            ctrl |= WM_LL_I2C_CR_NAK; /* el ultimo byte sin ACK */
            if (sendStop) {
                ctrl |= WM_LL_I2C_CR_STO;
            }
        }
        if (!readByte(&_rx[i], ctrl)) {
            stop();
            _rxLen = i;
            _rxIdx = 0;
            return i;
        }
    }
    _busHeld = !sendStop;
    _rxLen   = size;
    _rxIdx   = 0;
    return size;
}

size_t TwoWire::write(uint8_t data)
{
    if (!_txActive || _txLen >= I2C_BUFFER_LENGTH) {
        setWriteError();
        return 0;
    }
    _tx[_txLen++] = data;
    return 1;
}

size_t TwoWire::write(const uint8_t *data, size_t quantity)
{
    size_t n = 0;
    while (n < quantity && write(data[n])) {
        n++;
    }
    return n;
}

int TwoWire::available()
{
    return (int)(_rxLen - _rxIdx);
}

int TwoWire::read()
{
    if (_rxIdx >= _rxLen) {
        return -1;
    }
    return _rx[_rxIdx++];
}

int TwoWire::peek()
{
    if (_rxIdx >= _rxLen) {
        return -1;
    }
    return _rx[_rxIdx];
}

void TwoWire::flush()
{
    _rxLen = _rxIdx = 0;
    _txLen = 0;
}
