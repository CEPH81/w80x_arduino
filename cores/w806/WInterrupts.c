/*
 * WInterrupts.c - attachInterrupt/detachInterrupt para W80x
 * (sobre el servicio de interrupciones de GPIO del SDK).
 *
 * Parte de w80x_arduino. LGPL 2.1.
 */
#include "Arduino.h"
#include "wiring_private.h"

#include "wm_drv_gpio.h"

typedef struct {
    voidFuncPtrArg fn;
    void *arg;
    bool with_arg;
} isr_slot_t;

static isr_slot_t isr_slots[W80X_PIN_MUX_COUNT];

static void gpio_isr(void *p)
{
    isr_slot_t *s = (isr_slot_t *)p;
    if (s->fn) {
        if (s->with_arg) {
            s->fn(s->arg);
        } else {
            ((voidFuncPtr)s->fn)();
        }
    }
}

static void attach(uint8_t pin, voidFuncPtrArg fn, void *arg, bool with_arg, int mode)
{
    if (!w80x_pin_valid(pin) || fn == NULL || mode < FALLING || mode > ONHIGH) {
        return;
    }
    if (w80x_pin_state[pin] != W80X_PIN_GPIO) {
        /* si no se configuro con pinMode, queda como entrada */
        w80x_pin_to_gpio(pin, false);
    }
    wm_gpio_num_t g = (wm_gpio_num_t)pin;
    wm_drv_gpio_disable_isr(g);
    wm_drv_gpio_remove_isr_callback(g);
    isr_slots[pin].fn       = fn;
    isr_slots[pin].arg      = arg;
    isr_slots[pin].with_arg = with_arg;
    wm_drv_gpio_set_intr_mode(g, (wm_gpio_intr_mode_t)mode);
    wm_drv_gpio_add_isr_callback(g, gpio_isr, &isr_slots[pin]);
    wm_drv_gpio_enable_isr(g);
}

void attachInterrupt(uint8_t pin, voidFuncPtr callback, int mode)
{
    attach(pin, (voidFuncPtrArg)callback, NULL, false, mode);
}

void attachInterruptArg(uint8_t pin, voidFuncPtrArg callback, void *arg, int mode)
{
    attach(pin, callback, arg, true, mode);
}

void detachInterrupt(uint8_t pin)
{
    if (!w80x_pin_valid(pin)) {
        return;
    }
    wm_drv_gpio_disable_isr((wm_gpio_num_t)pin);
    wm_drv_gpio_remove_isr_callback((wm_gpio_num_t)pin);
    isr_slots[pin].fn = NULL;
}
