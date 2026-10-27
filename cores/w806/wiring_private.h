/*
 * wiring_private.h - funciones internas del nucleo W80x.
 *
 * Parte de w80x_arduino. LGPL 2.1.
 */
#ifndef WiringPrivate_h
#define WiringPrivate_h

#include <stdint.h>
#include <stdbool.h>
#include "pins_mux.h"

#ifdef __cplusplus
extern "C" {
#endif

/* uso actual de cada pata */
enum {
    W80X_PIN_UNUSED = 0,
    W80X_PIN_GPIO,
    W80X_PIN_PWM,
    W80X_PIN_ADC,
    W80X_PIN_PERIPH, /* UART, I2C, SPI... */
};

extern uint8_t w80x_pin_state[W80X_PIN_MUX_COUNT];

static inline bool w80x_pin_valid(uint8_t pin)
{
    return pin < W80X_PIN_MUX_COUNT && w80x_pin_mux[pin].valid;
}

/* pasa la pata a GPIO (salida o entrada) sin tocar el nivel */
void w80x_pin_to_gpio(uint8_t pin, bool output);
/* detiene el PWM de la pata si lo tiene */
void w80x_pwm_detach(uint8_t pin);
/* inicializacion del nucleo (antes de setup) */
void w80x_init_digital(void);

#ifdef __cplusplus
}
#endif

#endif
