/*
 * SPI.cpp - bus SPI (maestro) del W80x sobre el driver spim del SDK.
 *
 * Parte de w80x_arduino. LGPL 2.1.
 */
#include "SPI.h"
#include "wiring_private.h"

extern "C" {
#include "wm_drv_spi_master.h"
#include "wm_dt_hw.h"
}

#define BOUNCE 64 /* buffer en RAM para datos que estan en flash o para LSBFIRST */

SPIClass SPI;

static inline uint8_t rev8(uint8_t b)
{
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

SPIClass::SPIClass()
    : _sck(PIN_SPI_SCK), _miso(PIN_SPI_MISO), _mosi(PIN_SPI_MOSI), _ss(PIN_SPI_SS), _started(false),
      _inTransaction(false), _dev(nullptr), _lock(nullptr)
{
}

void SPIClass::begin()
{
    begin(_sck, _miso, _mosi, _ss);
}

void SPIClass::begin(int8_t sck, int8_t miso, int8_t mosi, int8_t ss)
{
    if (_started) {
        return;
    }
    if (_lock == nullptr) {
        _lock = xSemaphoreCreateRecursiveMutex();
    }
    wm_device_t *dev = wm_dt_get_device_by_name("spim");
    if (dev == nullptr) {
        return;
    }
    if (dev->state != WM_DEV_ST_INITED) {
        dev = wm_drv_spim_init("spim");
        if (dev == nullptr) {
            return;
        }
    }
    /* patas: las del arbol de dispositivos son SCK/MISO/MOSI de la placa;
       si se piden otras, se cambian */
    wm_dt_hw_spim_t *hw = (wm_dt_hw_spim_t *)dev->hw;
    for (int i = 0; i < hw->pin_cfg_count; i++) {
        uint8_t p = hw->pin_cfg[i].pin_num;
        if (p != sck && p != miso && p != mosi) {
            w80x_pin_release(p);
        } else if (p < W80X_PIN_MUX_COUNT) {
            w80x_pin_state[p] = W80X_PIN_PERIPH;
        }
    }
    if (sck >= 0) w80x_pin_attach_func(sck, W80X_F_SPI_CK);
    if (miso >= 0) w80x_pin_attach_func(miso, W80X_F_SPI_MISO);
    if (mosi >= 0) w80x_pin_attach_func(mosi, W80X_F_SPI_MOSI);
    _sck  = sck;
    _miso = miso;
    _mosi = mosi;
    _ss   = ss;
    _dev  = dev;
    _started = true;
}

void SPIClass::end()
{
    if (!_started) {
        return;
    }
    wm_drv_spim_deinit((wm_device_t *)_dev);
    if (_sck >= 0) w80x_pin_release(_sck);
    if (_miso >= 0) w80x_pin_release(_miso);
    if (_mosi >= 0) w80x_pin_release(_mosi);
    _dev     = nullptr;
    _started = false;
}

void SPIClass::beginTransaction(SPISettings settings)
{
    if (!_started) {
        begin();
    }
    if (_lock) {
        xSemaphoreTakeRecursive(_lock, portMAX_DELAY);
    }
    _settings      = settings;
    _inTransaction = true;
}

void SPIClass::endTransaction()
{
    if (_inTransaction) {
        _inTransaction = false;
        if (_lock) {
            xSemaphoreGiveRecursive(_lock);
        }
    }
}

void SPIClass::setFrequency(uint32_t freq)
{
    _settings._clock = freq;
}

bool SPIClass::xfer(const uint8_t *tx, uint8_t *rx, size_t len)
{
    if (!_started) {
        begin();
    }
    if (_dev == nullptr || len == 0) {
        return false;
    }
    uint32_t f = _settings._clock;
    if (f > WM_SPIM_MAX_CLOCK) f = WM_SPIM_MAX_CLOCK;
    if (f < WM_SPIM_MIN_CLOCK) f = WM_SPIM_MIN_CLOCK;

    wm_dt_hw_spim_dev_cfg_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.mode           = _settings._dataMode & 3;
    cfg.freq           = f;
    cfg.pin_cs.pin_num = WM_GPIO_NUM_MAX; /* CS lo maneja el programa */

    bool lsb = (_settings._bitOrder == LSBFIRST);
    uint8_t txb[BOUNCE];
    bool ok = true;
    while (len > 0 && ok) {
        size_t n = len;
        const uint8_t *t = tx;
        /* datos en flash (DMA no los lee) o LSB primero: se copian a RAM */
        bool copy = tx && (lsb || ((uint32_t)tx >= 0x08000000UL && (uint32_t)tx < 0x10000000UL));
        if (copy) {
            if (n > BOUNCE) n = BOUNCE;
            for (size_t i = 0; i < n; i++) {
                txb[i] = lsb ? rev8(tx[i]) : tx[i];
            }
            t = txb;
        }
        spim_transceive_t d;
        memset(&d, 0, sizeof(d));
        d.tx_buf = (uint8_t *)t;
        d.tx_len = t ? n : 0;
        d.rx_buf = rx;
        d.rx_len = rx ? n : 0;
        if (t == nullptr) {
            /* solo lectura: se envia 0xFF */
            memset(txb, 0xFF, sizeof(txb));
            if (n > BOUNCE) n = BOUNCE;
            d.tx_buf = txb;
            d.tx_len = n;
            d.rx_len = n;
        }
        ok = wm_drv_spim_transceive_sync((wm_device_t *)_dev, &cfg, &d, 1000) == WM_ERR_SUCCESS;
        if (rx && lsb) {
            for (size_t i = 0; i < n; i++) {
                rx[i] = rev8(rx[i]);
            }
        }
        if (tx) tx += n;
        if (rx) rx += n;
        len -= n;
    }
    return ok;
}

uint8_t SPIClass::transfer(uint8_t data)
{
    uint8_t r = 0;
    xfer(&data, &r, 1);
    return r;
}

uint16_t SPIClass::transfer16(uint16_t data)
{
    uint8_t t[2], r[2] = { 0, 0 };
    if (_settings._bitOrder == LSBFIRST) {
        t[0] = data & 0xFF;
        t[1] = data >> 8;
        xfer(t, r, 2);
        return r[0] | (r[1] << 8);
    }
    t[0] = data >> 8;
    t[1] = data & 0xFF;
    xfer(t, r, 2);
    return (r[0] << 8) | r[1];
}

uint32_t SPIClass::transfer32(uint32_t data)
{
    uint8_t t[4], r[4] = { 0, 0, 0, 0 };
    bool lsb = _settings._bitOrder == LSBFIRST;
    for (int i = 0; i < 4; i++) {
        t[i] = lsb ? (data >> (8 * i)) : (data >> (24 - 8 * i));
    }
    xfer(t, r, 4);
    uint32_t v = 0;
    for (int i = 0; i < 4; i++) {
        v |= (uint32_t)r[i] << (lsb ? 8 * i : 24 - 8 * i);
    }
    return v;
}

void SPIClass::transfer(void *buf, size_t count)
{
    /* se envia y se recibe sobre el mismo buffer */
    uint8_t *p = (uint8_t *)buf;
    uint8_t tmp[BOUNCE];
    while (count > 0) {
        size_t n = count > BOUNCE ? BOUNCE : count;
        memcpy(tmp, p, n);
        xfer(tmp, p, n);
        p += n;
        count -= n;
    }
}

void SPIClass::transferBytes(const uint8_t *data, uint8_t *out, uint32_t size)
{
    xfer(data, out, size);
}

void SPIClass::writeBytes(const uint8_t *data, uint32_t size)
{
    xfer(data, nullptr, size);
}
