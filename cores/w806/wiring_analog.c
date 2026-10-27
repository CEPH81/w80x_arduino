/*
 * wiring_analog.c - analogRead (ADC) y analogWrite (PWM) para W80x.
 *
 * ADC: 4 canales (2 en W800/Air101), entrada de 0 a 2,4 V. El SDK aplica la
 *      calibracion de fabrica y entrega milivoltios; analogRead() los escala
 *      a la resolucion elegida (12 bits por defecto: 0..4095).
 * PWM: 5 canales de 8 bits. Cada canal puede salir por varias patas (ver
 *      pins_mux.h); las patas que comparten canal llevan la misma senal.
 *
 * Parte de w80x_arduino. LGPL 2.1.
 */
#include "Arduino.h"
#include "wiring_private.h"

#include "wm_drv_gpio.h"
#include "wm_drv_adc.h"
#include "wm_drv_pwm.h"

#define ADC_FULL_SCALE_MV 2400
#define PWM_CLOCK_HZ      40000000UL /* reloj de perifericos (fijo) */
#define PWM_PERIOD        254        /* 255 pasos: duty/255 */

static wm_device_t *adc_dev;
static uint8_t adc_bits = 12;

static wm_device_t *pwm_dev;
static uint8_t pwm_bits    = 8;
static uint32_t pwm_freq   = 1000;
static uint8_t pwm_running; /* bit n: canal n configurado */

int w80x_pin_pwm_channel(uint8_t pin)
{
    if (!w80x_pin_valid(pin) || w80x_pin_mux[pin].pwm_ch == 0xF) {
        return -1;
    }
    return w80x_pin_mux[pin].pwm_ch;
}

int w80x_pin_adc_channel(uint8_t pin)
{
    if (!w80x_pin_valid(pin) || w80x_pin_mux[pin].adc_ch == 0xF) {
        return -1;
    }
    return w80x_pin_mux[pin].adc_ch;
}

/* ------------------------------------------------------------------ ADC */

static wm_device_t *adc_get(void)
{
    if (adc_dev == NULL) {
        adc_dev = wm_drv_adc_init("adc");
        if (adc_dev == NULL) {
            adc_dev = wm_dt_get_device_by_name("adc"); /* ya iniciado */
        }
    }
    return adc_dev;
}

void analogReadResolution(uint8_t bits)
{
    if (bits >= 1 && bits <= 16) {
        adc_bits = bits;
    }
}

void analogReference(uint8_t mode)
{
    (void)mode;
}

uint32_t analogReadMilliVolts(uint8_t pin)
{
    int ch = w80x_pin_adc_channel(pin);
    if (ch < 0) {
        /* tambien se acepta el numero de canal (0..3) */
        if (pin < 4 && !w80x_pin_valid(pin)) {
            ch = pin;
        } else {
            return 0;
        }
    } else if (w80x_pin_state[pin] != W80X_PIN_ADC) {
        if (w80x_pin_state[pin] == W80X_PIN_PWM) {
            w80x_pwm_detach(pin);
        }
        wm_drv_gpio_iomux_func_sel((wm_gpio_num_t)pin, WM_GPIO_IOMUX_FUN6);
        w80x_pin_state[pin] = W80X_PIN_ADC;
    }
    wm_device_t *dev = adc_get();
    int mv           = 0;
    if (dev == NULL || wm_drv_adc_oneshot(dev, (wm_adc_channel_t)ch, &mv) != WM_ERR_SUCCESS) {
        return 0;
    }
    return mv < 0 ? 0 : (uint32_t)mv;
}

int analogRead(uint8_t pin)
{
    uint32_t mv  = analogReadMilliVolts(pin);
    uint32_t top = (1UL << adc_bits) - 1;
    if (mv >= ADC_FULL_SCALE_MV) {
        return top;
    }
    return (int)((mv * top + ADC_FULL_SCALE_MV / 2) / ADC_FULL_SCALE_MV);
}

float temperatureRead(void)
{
    wm_device_t *dev = adc_get();
    int t            = 0;
    if (dev == NULL || wm_drv_thermal_sensor_read_temperature(dev, &t) != WM_ERR_SUCCESS) {
        return NAN;
    }
    return t / 1000.0f;
}

/* ------------------------------------------------------------------ PWM */

static wm_device_t *pwm_get(void)
{
    if (pwm_dev == NULL) {
        pwm_dev = wm_drv_pwm_init("pwm");
        if (pwm_dev == NULL) {
            pwm_dev = wm_dt_get_device_by_name("pwm");
        }
    }
    return pwm_dev;
}

static uint16_t pwm_clkdiv(void)
{
    uint32_t div = (PWM_CLOCK_HZ + pwm_freq * (PWM_PERIOD + 1) / 2) / (pwm_freq * (PWM_PERIOD + 1));
    if (div < 1) {
        div = 1;
    } else if (div > 0xFFFF) {
        div = 0xFFFF;
    }
    return (uint16_t)div;
}

void analogWriteResolution(uint8_t bits)
{
    if (bits >= 1 && bits <= 16) {
        pwm_bits = bits;
    }
}

void analogWriteFrequency(uint32_t freq)
{
    if (freq == 0) {
        return;
    }
    pwm_freq = freq;
    if (pwm_dev) {
        uint16_t div = pwm_clkdiv();
        for (uint8_t ch = 0; ch < 5; ch++) {
            if (pwm_running & (1 << ch)) {
                wm_drv_pwm_set_channel_period_clkdiv(pwm_dev, ch, PWM_PERIOD, div);
            }
        }
    }
}

/* apaga el canal si ninguna otra pata lo usa */
void w80x_pwm_detach(uint8_t pin)
{
    int ch = w80x_pin_pwm_channel(pin);
    w80x_pin_state[pin] = W80X_PIN_UNUSED;
    if (ch < 0 || pwm_dev == NULL) {
        return;
    }
    for (uint8_t p = 0; p < W80X_PIN_MUX_COUNT; p++) {
        if (w80x_pin_state[p] == W80X_PIN_PWM && w80x_pin_mux[p].pwm_ch == ch) {
            return;
        }
    }
    wm_drv_pwm_channel_stop(pwm_dev, ch);
    wm_drv_pwm_channel_deinit(pwm_dev, ch);
    pwm_running &= ~(1 << ch);
}

void analogWrite(uint8_t pin, int value)
{
    int ch = w80x_pin_pwm_channel(pin);
    if (ch < 0) {
        /* sin PWM: como en Arduino, nivel bajo o alto */
        if (w80x_pin_valid(pin)) {
            digitalWrite(pin, value >= (1 << pwm_bits) / 2 ? HIGH : LOW);
        }
        return;
    }

    uint32_t top = (1UL << pwm_bits) - 1;
    if (value <= 0 || (uint32_t)value >= top) {
        /* 0 % y 100 %: la pata pasa a nivel fijo */
        if (w80x_pin_state[pin] != W80X_PIN_GPIO) {
            w80x_pin_to_gpio(pin, true);
        }
        digitalWrite(pin, value <= 0 ? LOW : HIGH);
        return;
    }

    /* 1..254 de 255 */
    uint32_t duty = ((uint32_t)value * (PWM_PERIOD + 1) + top / 2) / top;
    if (duty < 1) {
        duty = 1;
    } else if (duty > PWM_PERIOD) {
        duty = PWM_PERIOD;
    }

    wm_device_t *dev = pwm_get();
    if (dev == NULL) {
        return;
    }
    if (!(pwm_running & (1 << ch))) {
        wm_drv_pwm_channel_cfg_t cfg = { 0 };
        cfg.channel      = (enum wm_pwm_channels)ch;
        cfg.mode         = WM_PWM_OUT_INDPT;
        cfg.clkdiv       = pwm_clkdiv();
        cfg.period_cycle = PWM_PERIOD;
        cfg.duty_cycle   = duty - 1;
        cfg.autoload     = true;
        if (wm_drv_pwm_channel_init(dev, &cfg) != WM_ERR_SUCCESS) {
            return;
        }
        wm_drv_pwm_channel_start(dev, ch);
        pwm_running |= 1 << ch;
    } else {
        wm_drv_pwm_set_channel_duty(dev, ch, duty - 1);
    }
    if (w80x_pin_state[pin] != W80X_PIN_PWM) {
        wm_drv_gpio_iomux_func_sel((wm_gpio_num_t)pin, (wm_gpio_pin_mux_t)w80x_pin_mux[pin].pwm_fun);
        w80x_pin_state[pin] = W80X_PIN_PWM;
    }
}
