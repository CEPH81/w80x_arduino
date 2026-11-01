/*
 * WInterrupts.c - attachInterrupt/detachInterrupt para W80x.
 *
 * El despachador de interrupciones de GPIO es propio y va en RAM: lee el
 * registro de interrupciones pendientes y solo atiende esas patas (el del
 * SDK revisa las 44 patas una a una y corre desde la flash).
 *
 * Parte de w80x_arduino. LGPL 2.1.
 */
#include "Arduino.h"
#include "wiring_private.h"

#include "wm_drv_gpio.h"
#include "wm_hal_irq.h"
#include "wm_gpio_reg_struct.h"
#include "wm_attr.h"
#include "csi_core.h"

typedef struct {
    voidFuncPtrArg fn;
    void *arg;
    bool with_arg;
} isr_slot_t;

static isr_slot_t isr_slots[W80X_PIN_MUX_COUNT];
static bool dispatcher_installed;

ATTRIBUTE_IRAM static void gpio_dispatch(wm_irq_no_t irq, void *arg)
{
    uint32_t base_pin  = (uint32_t)arg;
    wm_gpio_reg_t *reg = (wm_gpio_reg_t *)(base_pin ? W80X_GPIOB_BASE : W80X_GPIOA_BASE);
    uint32_t pending   = reg->mis;
    reg->ic            = pending; /* borrar las que se van a atender */
    (void)irq;
    while (pending) {
        uint32_t bit = __builtin_ctz(pending);
        pending &= pending - 1;
        uint32_t pin = base_pin + bit;
        if (pin < W80X_PIN_MUX_COUNT) {
            isr_slot_t *s = &isr_slots[pin];
            if (s->fn) {
                if (s->with_arg) {
                    s->fn(s->arg);
                } else {
                    ((voidFuncPtr)s->fn)();
                }
            }
        }
    }
}

static void install_dispatcher(void)
{
    if (!dispatcher_installed) {
        /* sustituye al despachador del SDK (lo instala wm_drv_gpio_init) */
        wm_hal_irq_attach_sw_vector(WM_IRQ_GPIOA, gpio_dispatch, (void *)0);
        wm_hal_irq_attach_sw_vector(WM_IRQ_GPIOB, gpio_dispatch, (void *)16);
        wm_hal_irq_enable(WM_IRQ_GPIOA);
        wm_hal_irq_enable(WM_IRQ_GPIOB);
        dispatcher_installed = true;
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
    install_dispatcher();
    wm_gpio_num_t g = (wm_gpio_num_t)pin;
    wm_drv_gpio_disable_isr(g);
    uint32_t flags          = csi_irq_save();
    isr_slots[pin].fn       = fn;
    isr_slots[pin].arg      = arg;
    isr_slots[pin].with_arg = with_arg;
    csi_irq_restore(flags);
    wm_drv_gpio_set_intr_mode(g, (wm_gpio_intr_mode_t)mode);
    /* borrar un flanco viejo antes de activar */
    ((wm_gpio_reg_t *)(pin < 16 ? W80X_GPIOA_BASE : W80X_GPIOB_BASE))->ic = 1UL << (pin < 16 ? pin : pin - 16);
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
    isr_slots[pin].fn = NULL;
}
