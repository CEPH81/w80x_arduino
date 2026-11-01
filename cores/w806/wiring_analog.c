/*
 * wiring_analog.c - analogRead (ADC) y analogWrite (PWM) para W80x.
 *
 * ADC: 4 canales (2 en W800/Air101). Entrada de 0 a 2,4 V como maximo
 *      (no aplicar mas: use un divisor para tensiones mayores).
 *      El SDK aplica la calibracion de fabrica (si el chip la trae) y entrega
 *      milivoltios. Encima se puede aplicar una calibracion propia de dos
 *      puntos (analogCalibrate), que se guarda en la flash.
 *      analogRead() escala 0..2400 mV a la resolucion elegida (12 bits: 0..4095).
 * PWM: 5 canales. El periodo es de 8 bits (255 pasos) hasta 156 kHz; por encima
 *      se usan menos pasos (hasta 20 MHz con 2 pasos). Las patas que comparten
 *      canal llevan la misma senal (ver pins_mux.h).
 *
 * Parte de w80x_arduino. LGPL 2.1.
 */
#include "Arduino.h"
#include "wiring_private.h"

#include "wm_drv_gpio.h"
#include "wm_drv_adc.h"
#include "wm_drv_pwm.h"
#include "wm_nvs.h"
#include "wm_ft_param.h"

#define ADC_FULL_SCALE_MV 2400
#define ADC_MAX_AVG       64
#define PWM_CLOCK_HZ      40000000UL /* reloj de perifericos (fijo) */
#define PWM_MAX_PERIOD    254        /* 255 pasos */

static wm_device_t *adc_dev;
static uint8_t adc_bits = 12;
static uint8_t adc_avg  = 1;

/* calibracion propia: mV_reales = mV_leidos * gain / 65536 + offset */
static bool cal_loaded;
static int32_t cal_gain = 65536;
static int32_t cal_offset;

static wm_device_t *pwm_dev;
static uint8_t pwm_bits = 8;
static uint32_t pwm_freq = 1000;
static uint8_t pwm_period = PWM_MAX_PERIOD;
static uint16_t pwm_clkdiv = 157;
static uint8_t pwm_running;   /* bit n: canal n configurado */
static uint8_t pwm_duty[5];   /* ultimo duty escrito en cada canal */
static uint32_t pwm_value[5]; /* ultimo valor pedido (para cambiar de frecuencia) */

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
    if (!cal_loaded) {
        cal_loaded = true;
        wm_nvs_handle_t h;
        if (wm_nvs_open(WM_NVS_DEF_PARTITION, "adc_cal", WM_NVS_OP_READ_ONLY, &h) == WM_ERR_SUCCESS) {
            int32_t g = 0, o = 0;
            if (wm_nvs_get_i32(h, "gain", &g) == WM_ERR_SUCCESS && wm_nvs_get_i32(h, "offset", &o) == WM_ERR_SUCCESS &&
                g > 32768 && g < 131072) {
                cal_gain   = g;
                cal_offset = o;
            }
            wm_nvs_close(h);
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

void analogReadAveraging(uint8_t samples)
{
    adc_avg = samples < 1 ? 1 : (samples > ADC_MAX_AVG ? ADC_MAX_AVG : samples);
}

void analogReference(uint8_t mode)
{
    (void)mode; /* referencia interna fija */
}

bool analogFactoryCalibrated(void)
{
    wm_ft_adc_cal_t c;
    memset(&c, 0, sizeof(c));
    return wm_ft_get_adc_cal_param(&c) == WM_ERR_SUCCESS && c.valid_cnt >= 2;
}

static int adc_channel_for(uint8_t pin)
{
    int ch = w80x_pin_adc_channel(pin);
    if (ch < 0) {
        /* tambien se acepta el numero de canal (0..3) */
        return (pin < 4 && !w80x_pin_valid(pin)) ? pin : -1;
    }
    if (w80x_pin_state[pin] != W80X_PIN_ADC) {
        if (w80x_pin_state[pin] == W80X_PIN_PWM) {
            w80x_pwm_detach(pin);
        }
        wm_drv_gpio_iomux_func_sel((wm_gpio_num_t)pin, WM_GPIO_IOMUX_FUN6);
        w80x_pin_state[pin] = W80X_PIN_ADC;
    }
    return ch;
}

/* lectura sin la calibracion propia (solo la del SDK), en mV */
static int32_t adc_raw_mv(int ch)
{
    wm_device_t *dev = adc_get();
    if (dev == NULL) {
        return 0;
    }
    if (adc_avg <= 1) {
        int mv = 0;
        if (wm_drv_adc_oneshot(dev, (wm_adc_channel_t)ch, &mv) != WM_ERR_SUCCESS) {
            return 0;
        }
        return mv;
    }
    /* varias muestras con una sola configuracion del ADC (mas rapido que varias lecturas) */
    int buf[ADC_MAX_AVG];
    if (wm_drv_adc_polling(dev, (wm_adc_channel_t)ch, buf, adc_avg, 1000) != WM_ERR_SUCCESS) {
        return 0;
    }
    int32_t sum = 0;
    for (int i = 0; i < adc_avg; i++) {
        sum += buf[i];
    }
    return (sum + adc_avg / 2) / adc_avg;
}

uint32_t analogReadMilliVolts(uint8_t pin)
{
    int ch = adc_channel_for(pin);
    if (ch < 0) {
        return 0;
    }
    int32_t mv = adc_raw_mv(ch);
    mv         = (int32_t)(((int64_t)mv * cal_gain) >> 16) + cal_offset;
    if (mv < 0) {
        mv = 0;
    } else if (mv > ADC_FULL_SCALE_MV) {
        mv = ADC_FULL_SCALE_MV; /* fuera de rango: la entrada no debe pasar de 2,4 V */
    }
    return (uint32_t)mv;
}

int analogRead(uint8_t pin)
{
    uint32_t mv  = analogReadMilliVolts(pin);
    uint32_t top = (1UL << adc_bits) - 1;
    return (int)((mv * top + ADC_FULL_SCALE_MV / 2) / ADC_FULL_SCALE_MV);
}

/* lectura para calibrar: mV con la calibracion de fabrica, sin la propia */
uint32_t analogReadUncalibratedMilliVolts(uint8_t pin)
{
    int ch = adc_channel_for(pin);
    if (ch < 0) {
        return 0;
    }
    int32_t mv = adc_raw_mv(ch);
    return mv < 0 ? 0 : (uint32_t)mv;
}

/* calibracion de dos puntos: con dos tensiones conocidas (real) y lo que
   devolvio analogReadUncalibratedMilliVolts (leido). Se guarda en la flash. */
bool analogCalibrate(uint32_t read1_mv, uint32_t real1_mv, uint32_t read2_mv, uint32_t real2_mv)
{
    if (read1_mv == read2_mv) {
        return false;
    }
    int64_t g = ((int64_t)((int32_t)real2_mv - (int32_t)real1_mv) << 16) / ((int32_t)read2_mv - (int32_t)read1_mv);
    if (g < 32768 || g > 131072) {
        return false; /* mas de un 50 % de error: medidas equivocadas */
    }
    int32_t o = (int32_t)real1_mv - (int32_t)(((int64_t)read1_mv * g) >> 16);
    cal_gain   = (int32_t)g;
    cal_offset = o;
    cal_loaded = true;
    wm_nvs_handle_t h;
    if (wm_nvs_open(WM_NVS_DEF_PARTITION, "adc_cal", WM_NVS_OP_READ_WRITE, &h) != WM_ERR_SUCCESS) {
        return false;
    }
    bool ok = wm_nvs_set_i32(h, "gain", cal_gain) == WM_ERR_SUCCESS && wm_nvs_set_i32(h, "offset", cal_offset) == WM_ERR_SUCCESS;
    wm_nvs_close(h);
    return ok;
}

void analogCalibrationReset(void)
{
    cal_gain   = 65536;
    cal_offset = 0;
    cal_loaded = true;
    wm_nvs_handle_t h;
    if (wm_nvs_open(WM_NVS_DEF_PARTITION, "adc_cal", WM_NVS_OP_READ_WRITE, &h) == WM_ERR_SUCCESS) {
        wm_nvs_del_group(h);
        wm_nvs_close(h);
    }
}

float temperatureRead(void)
{
    wm_device_t *dev = adc_get();
    if (dev == NULL) {
        return NAN;
    }
    int32_t sum = 0;
    int n       = 0;
    for (int i = 0; i < 4; i++) { /* promedio de 4 lecturas */
        int t = 0;
        if (wm_drv_thermal_sensor_read_temperature(dev, &t) == WM_ERR_SUCCESS) {
            sum += t;
            n++;
        }
    }
    return n ? (sum / n) / 1000.0f : NAN;
}

uint32_t analogReadVddMilliVolts(void)
{
    wm_device_t *dev = adc_get();
    int mv           = 0;
    if (dev == NULL || wm_drv_adc_chip_voltage_read_vdd(dev, &mv) != WM_ERR_SUCCESS) {
        return 0;
    }
    return mv < 0 ? 0 : (uint32_t)mv;
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

/* periodo y divisor para la frecuencia pedida: la mayor resolucion posible */
static void pwm_timing(void)
{
    uint32_t steps = PWM_CLOCK_HZ / pwm_freq; /* ciclos de 40 MHz por periodo */
    if (steps >= PWM_MAX_PERIOD + 1) {
        pwm_period = PWM_MAX_PERIOD;
        uint32_t div = (PWM_CLOCK_HZ + pwm_freq * (PWM_MAX_PERIOD + 1) / 2) / (pwm_freq * (PWM_MAX_PERIOD + 1));
        pwm_clkdiv   = div < 1 ? 1 : (div > 0xFFFF ? 0xFFFF : div);
    } else {
        if (steps < 2) {
            steps = 2;
        }
        pwm_period = steps - 1;
        pwm_clkdiv = 1;
    }
}

uint32_t analogWriteGetFrequency(void)
{
    return PWM_CLOCK_HZ / ((uint32_t)(pwm_period + 1) * pwm_clkdiv);
}

void analogWriteResolution(uint8_t bits)
{
    if (bits >= 1 && bits <= 16) {
        pwm_bits = bits;
    }
}

/* pasos activos (0..period+1) para un valor de 0..top */
static uint32_t pwm_steps(uint32_t value, uint32_t top)
{
    return (value * (uint32_t)(pwm_period + 1) + top / 2) / top;
}

static void pwm_apply(int ch, uint32_t steps)
{
    uint8_t duty = (uint8_t)(steps - 1);
    if (!(pwm_running & (1 << ch))) {
        wm_drv_pwm_channel_cfg_t cfg = { 0 };
        cfg.channel      = (enum wm_pwm_channels)ch;
        cfg.mode         = WM_PWM_OUT_INDPT;
        cfg.clkdiv       = pwm_clkdiv;
        cfg.period_cycle = pwm_period;
        cfg.duty_cycle   = duty;
        cfg.autoload     = true;
        if (wm_drv_pwm_channel_init(pwm_dev, &cfg) != WM_ERR_SUCCESS) {
            return;
        }
        wm_drv_pwm_channel_start(pwm_dev, ch);
        pwm_running |= 1 << ch;
    } else if (pwm_duty[ch] != duty) {
        wm_drv_pwm_set_channel_duty(pwm_dev, ch, duty);
    }
    pwm_duty[ch] = duty;
}

void analogWriteFrequency(uint32_t freq)
{
    if (freq == 0) {
        return;
    }
    if (freq > PWM_CLOCK_HZ / 2) {
        freq = PWM_CLOCK_HZ / 2;
    }
    pwm_freq = freq;
    pwm_timing();
    if (pwm_dev) {
        uint32_t top = (1UL << pwm_bits) - 1;
        for (uint8_t ch = 0; ch < 5; ch++) {
            if (pwm_running & (1 << ch)) {
                wm_drv_pwm_set_channel_period_clkdiv(pwm_dev, ch, pwm_period, pwm_clkdiv);
                uint32_t s = pwm_steps(pwm_value[ch], top);
                if (s < 1) s = 1;
                if (s > pwm_period) s = pwm_period;
                pwm_duty[ch] = 0xFF; /* forzar la escritura */
                pwm_apply(ch, s);
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
    uint32_t top = (1UL << pwm_bits) - 1;
    if (ch < 0) {
        /* sin PWM: como en Arduino, nivel bajo o alto */
        if (w80x_pin_valid(pin)) {
            digitalWrite(pin, (uint32_t)value >= (top + 1) / 2 ? HIGH : LOW);
        }
        return;
    }
    if (value < 0) {
        value = 0;
    } else if ((uint32_t)value > top) {
        value = top;
    }
    if (pwm_clkdiv == 157 && pwm_period == PWM_MAX_PERIOD) {
        pwm_timing(); /* primera vez */
    }
    uint32_t steps = pwm_steps(value, top);
    if (steps == 0 || steps > pwm_period) {
        /* 0 % y 100 %: la pata pasa a nivel fijo */
        if (w80x_pin_state[pin] != W80X_PIN_GPIO) {
            w80x_pin_to_gpio(pin, true);
        }
        digitalWrite(pin, steps == 0 ? LOW : HIGH);
        return;
    }
    if (pwm_get() == NULL) {
        return;
    }
    pwm_value[ch] = value;
    pwm_apply(ch, steps);
    if (w80x_pin_state[pin] != W80X_PIN_PWM) {
        wm_drv_gpio_iomux_func_sel((wm_gpio_num_t)pin, (wm_gpio_pin_mux_t)w80x_pin_mux[pin].pwm_fun);
        w80x_pin_state[pin] = W80X_PIN_PWM;
    }
}
