/*
 * HardwareTimer.cpp - temporizadores del W80x sobre el driver del SDK.
 * Parte de w80x_arduino. LGPL 2.1.
 */
#include "HardwareTimer.h"

extern "C" {
#include "wm_drv_timer.h"
#include "wm_error.h"
}

static void timer_isr(void *arg)
{
    ((HardwareTimer *)arg)->irq();
}

HardwareTimer::HardwareTimer(uint8_t timerNum)
    : _num(timerNum < TIMER_COUNT ? timerNum : 0), _unit(TIM_UNIT_US), _reload(1), _running(false), _period(1000),
      _cb(NULL), _dev(NULL)
{
}

bool HardwareTimer::open()
{
    if (_dev) {
        return true;
    }
    char name[8] = { 't', 'i', 'm', 'e', 'r', (char)('0' + _num), 0 };
    wm_device_t *d = wm_dt_get_device_by_name(name);
    if (d == NULL) {
        return false;
    }
    if (d->state != WM_DEV_ST_INITED) {
        d = wm_drv_timer_init(name);
        if (d == NULL) {
            return false;
        }
    }
    wm_drv_timer_register_callback(d, timer_isr, this);
    _dev = d;
    return true;
}

uint8_t HardwareTimer::configure(uint32_t period, uint32_t units, uint32_t flags, timer_irq_callback callback)
{
    if (!open() || period == 0) {
        return 1;
    }
    if (_running) {
        stop();
    }
    _period = period;
    _unit   = units ? TIM_UNIT_MS : TIM_UNIT_US;
    _reload = flags ? 1 : 0;
    if (callback) {
        _cb = callback;
    }
    return 0;
}

void HardwareTimer::irq()
{
    if (!_reload) {
        _running = false;
    }
    timer_irq_callback cb = _cb;
    if (cb) {
        cb();
    }
}

void HardwareTimer::start()
{
    if (!open()) {
        return;
    }
    wm_drv_timer_cfg_t cfg;
    cfg.unit        = _unit == TIM_UNIT_MS ? WM_HAL_TIMER_UNIT_MS : WM_HAL_TIMER_UNIT_US;
    cfg.period      = _period;
    cfg.auto_reload = _reload != 0;
    if (wm_drv_timer_start((wm_device_t *)_dev, cfg) == WM_ERR_SUCCESS) {
        _running = true;
    }
}

void HardwareTimer::stop()
{
    if (_dev) {
        wm_drv_timer_stop((wm_device_t *)_dev);
    }
    _running = false;
}

void HardwareTimer::setPeriod(uint32_t period)
{
    _period = period;
    if (_dev && _running) {
        wm_drv_timer_set_period((wm_device_t *)_dev, period);
    }
}

uint32_t HardwareTimer::getCount()
{
    uint32_t c = 0;
    if (_dev) {
        wm_drv_timer_get_counter((wm_device_t *)_dev, &c);
    }
    return c;
}
