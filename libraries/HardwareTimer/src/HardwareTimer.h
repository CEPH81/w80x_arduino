/*
 * HardwareTimer.h - temporizadores por hardware del W80x (0 a 5).
 *
 *   HardwareTimer t(2);
 *   t.configure(500, TIM_UNIT_MS, TIM_AUTORELOAD_PRELOAD_ENABLE, mi_funcion);
 *   t.start();
 *
 * La funcion se llama desde la interrupcion: debe ser corta (sin delay ni Serial).
 * Parte de w80x_arduino. LGPL 2.1.
 */
#ifndef _HARDWARETIMER_H_
#define _HARDWARETIMER_H_

#include <Arduino.h>

#define TIMER_COUNT 6
#define TIM_UNIT_US 0
#define TIM_UNIT_MS 1
#define TIM_AUTORELOAD_PRELOAD_DISABLE 0 /* una sola vez */
#define TIM_AUTORELOAD_PRELOAD_ENABLE  1 /* periodico */

typedef void (*timer_irq_callback)();

class HardwareTimer
{
public:
    explicit HardwareTimer(uint8_t timerNum);
    /* 0 si todo bien */
    uint8_t configure(uint32_t period, uint32_t units = TIM_UNIT_US, uint32_t flags = TIM_AUTORELOAD_PRELOAD_ENABLE,
                      timer_irq_callback callback = NULL);
    void attachInterrupt(timer_irq_callback callback) { _cb = callback; }
    void detachInterrupt() { _cb = NULL; }
    void start();
    void stop();
    void pause() { stop(); }
    void resume() { start(); }
    void setPeriod(uint32_t period);
    uint32_t getPeriod() { return _period; }
    uint32_t getCount();
    uint32_t getTimerUnits() { return _unit; }
    uint32_t getTimerReloadFlag() { return _reload; }
    bool isRunning() { return _running; }

    /* estilo microsegundos (como otras librerias) */
    void setOverflow(uint32_t us) { configure(us, TIM_UNIT_US, TIM_AUTORELOAD_PRELOAD_ENABLE, _cb); }

    void irq(); /* uso interno */

private:
    bool open();
    uint8_t _num;
    uint8_t _unit;
    uint8_t _reload;
    bool _running;
    uint32_t _period;
    volatile timer_irq_callback _cb;
    void *_dev;
};

#endif
