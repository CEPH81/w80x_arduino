/*
 * wiring_digital.c - pinMode, digitalWrite, digitalRead para W80x.
 *
 * pinMode() usa el driver del SDK; digitalWrite()/digitalRead() acceden
 * directamente a los registros (rapido y sin bloqueos, valen en interrupciones).
 *
 * Parte de w80x_arduino. LGPL 2.1.
 */
#include "Arduino.h"
#include "wiring_private.h"

#include "wm_drv_gpio.h"
#include "wm_gpio_reg_struct.h"
#include "csi_core.h"

uint8_t w80x_pin_state[W80X_PIN_MUX_COUNT];

static inline wm_gpio_reg_t *gpio_port(uint8_t pin)
{
    return (wm_gpio_reg_t *)(pin < 16 ? W80X_GPIOA_BASE : W80X_GPIOB_BASE);
}

static inline uint32_t gpio_mask(uint8_t pin)
{
    return 1UL << (pin < 16 ? pin : pin - 16);
}

void w80x_init_digital(void)
{
    /* servicio de interrupciones de GPIO del SDK (attachInterrupt) */
    wm_drv_gpio_init("gpio");
    /* PB19/PB20: UART0 (Serial, carga del programa) */
    w80x_pin_state[35] = W80X_PIN_PERIPH;
    w80x_pin_state[36] = W80X_PIN_PERIPH;
}

void w80x_pin_to_gpio(uint8_t pin, bool output)
{
    if (w80x_pin_state[pin] == W80X_PIN_PWM) {
        w80x_pwm_detach(pin);
    }
    wm_drv_gpio_iomux_func_sel((wm_gpio_num_t)pin, WM_GPIO_IOMUX_FUN5);
    wm_drv_gpio_set_dir((wm_gpio_num_t)pin, output ? WM_GPIO_DIR_OUTPUT : WM_GPIO_DIR_INPUT);
    w80x_pin_state[pin] = W80X_PIN_GPIO;
}

void pinMode(uint8_t pin, uint8_t mode)
{
    if (!w80x_pin_valid(pin)) {
        return;
    }
    switch (mode) {
        case OUTPUT:
            w80x_pin_to_gpio(pin, true);
            wm_drv_gpio_set_pullmode((wm_gpio_num_t)pin, WM_GPIO_FLOAT);
            break;
        case INPUT_PULLUP:
            w80x_pin_to_gpio(pin, false);
            wm_drv_gpio_set_pullmode((wm_gpio_num_t)pin, WM_GPIO_PULL_UP);
            break;
        case INPUT_PULLDOWN:
            w80x_pin_to_gpio(pin, false);
            wm_drv_gpio_set_pullmode((wm_gpio_num_t)pin, WM_GPIO_PULL_DOWN);
            break;
        case ANALOG_INPUT:
            if (w80x_pin_mux[pin].adc_ch != 0xF) {
                if (w80x_pin_state[pin] == W80X_PIN_PWM) {
                    w80x_pwm_detach(pin);
                }
                wm_drv_gpio_iomux_func_sel((wm_gpio_num_t)pin, WM_GPIO_IOMUX_FUN6);
                w80x_pin_state[pin] = W80X_PIN_ADC;
            }
            break;
        case PWM_OUT:
            /* el PWM se activa con analogWrite() */
            break;
        case INPUT:
        default:
            w80x_pin_to_gpio(pin, false);
            wm_drv_gpio_set_pullmode((wm_gpio_num_t)pin, WM_GPIO_FLOAT);
            break;
    }
}

void digitalWrite(uint8_t pin, uint8_t val)
{
    if (!w80x_pin_valid(pin)) {
        return;
    }
    uint8_t st = w80x_pin_state[pin];
    if (st == W80X_PIN_PWM || st == W80X_PIN_UNUSED) {
        /* como en AVR: si tenia PWM se apaga; si no se habia configurado, queda como salida */
        w80x_pin_to_gpio(pin, true);
    }
    wm_gpio_reg_t *port = gpio_port(pin);
    uint32_t mask       = gpio_mask(pin);
    uint32_t flags      = csi_irq_save();
    if (val) {
        port->data |= mask;
    } else {
        port->data &= ~mask;
    }
    csi_irq_restore(flags);
}

int digitalRead(uint8_t pin)
{
    if (!w80x_pin_valid(pin)) {
        return LOW;
    }
    return (gpio_port(pin)->data & gpio_mask(pin)) ? HIGH : LOW;
}

void digitalToggle(uint8_t pin)
{
    if (!w80x_pin_valid(pin)) {
        return;
    }
    uint8_t st = w80x_pin_state[pin];
    if (st == W80X_PIN_PWM || st == W80X_PIN_UNUSED) {
        w80x_pin_to_gpio(pin, true);
    }
    wm_gpio_reg_t *port = gpio_port(pin);
    uint32_t flags      = csi_irq_save();
    port->data ^= gpio_mask(pin);
    csi_irq_restore(flags);
}

int w80x_pin_func_fun(uint8_t pin, uint8_t func)
{
    for (unsigned i = 0; i < W80X_PIN_FUNCS_COUNT; i++) {
        if (w80x_pin_funcs[i].pin == pin && w80x_pin_funcs[i].func == func) {
            return w80x_pin_funcs[i].fun;
        }
    }
    return -1;
}

bool w80x_pin_attach_func(uint8_t pin, uint8_t func)
{
    int fun = w80x_pin_func_fun(pin, func);
    if (fun < 0 || !w80x_pin_valid(pin)) {
        return false;
    }
    if (w80x_pin_state[pin] == W80X_PIN_PWM) {
        w80x_pwm_detach(pin);
    }
    wm_drv_gpio_iomux_func_sel((wm_gpio_num_t)pin, (wm_gpio_pin_mux_t)fun);
    w80x_pin_state[pin] = W80X_PIN_PERIPH;
    return true;
}

void w80x_pin_release(uint8_t pin)
{
    if (!w80x_pin_valid(pin)) {
        return;
    }
    w80x_pin_to_gpio(pin, false);
    wm_drv_gpio_set_pullmode((wm_gpio_num_t)pin, WM_GPIO_FLOAT);
    w80x_pin_state[pin] = W80X_PIN_UNUSED;
}
