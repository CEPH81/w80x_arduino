#!/usr/bin/env python3
"""
gen_variants.py - Genera, para cada placa (carpeta variants/<placa>):

  wm_dt_hw.c   arbol de dispositivos del WM IoT SDK adaptado al nucleo Arduino:
               - frecuencia de CPU tomada de F_CPU (menu "CPU Frequency")
               - sin pines de ejemplo en GPIO, PWM y ADC (el nucleo asigna
                 cada pata al usarla: pinMode, analogWrite, analogRead)
               - los 4 canales de ADC disponibles (2 en el W800)
  pins_mux.h   tabla de funciones de cada pata (canal PWM, canal ADC y numero
               de funcion del multiplexor), sacada de la documentacion del SDK.

Uso:
    WM_IOT_SDK_PATH=/ruta/wm_iot_sdk python3 tools/sdk_builder/gen_variants.py

Necesita haber ejecutado antes build_sdk.py (lee sdk/<soc>/device_table).
Parte de w80x_arduino (WM IoT SDK). LGPL 2.1.
"""
import html, os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
PLAT = os.path.dirname(os.path.dirname(HERE))
SDK = os.environ.get('WM_IOT_SDK_PATH') or sys.exit('defina WM_IOT_SDK_PATH')

# placa -> chip del SDK
VARIANTS = {
    'w800': 'w800', 'air101': 'w800',
    'w801': 'w801s',
    'w802': 'w802',
    'w806': 'w806', 'air103': 'w806',
}
# orden de las tablas en docs/en/component_guides/pinmux.rst
TABLES = ['w800', 'w801s', 'w802', 'w803', 'w806']


def pinmux_tables():
    txt = open(os.path.join(SDK, 'docs', 'en', 'component_guides', 'pinmux.rst'), encoding='utf-8').read()
    out = {}
    for soc, tb in zip(TABLES, re.findall(r'<table.*?</table>', txt, re.S)):
        pins = {}
        for row in re.findall(r'<tr.*?</tr>', tb, re.S)[1:]:
            cells = [html.unescape(re.sub(r'<.*?>', '', c)).strip()
                     for c in re.findall(r'<t[dh].*?</t[dh]>', row, re.S)]
            num = int(cells[0].split('_')[-1])
            pins[num] = (cells[1], cells[2:9])
        out[soc] = pins
    return out


def gen_pins_mux(variant, soc, pins):
    n = max(pins) + 1
    lines = []
    for num in range(n):
        if num not in pins:
            lines.append('    { 0, 0, 0, 0 },          /* %2d: no existe */' % num)
            continue
        pad, funs = pins[num]
        pwm_ch = pwm_fun = adc_ch = 0xF
        for i, f in enumerate(funs):
            m = re.fullmatch(r'PWM(\d)', f)
            if m and pwm_ch == 0xF:
                pwm_ch, pwm_fun = int(m.group(1)), i + 1
            m = re.fullmatch(r'ADC_(\d)', f)
            if m:
                adc_ch = int(m.group(1))
        lines.append('    { 1, 0x%X, 0x%X, 0x%X },    /* %2d %-4s %s */' % (
            pwm_ch, pwm_fun, adc_ch, num, pad, ', '.join(x for x in funs if x)))
    body = '\n'.join(lines)
    return f'''/*
 * pins_mux.h - funciones de cada pata del {soc.upper()} ({variant})
 * Generado por tools/sdk_builder/gen_variants.py a partir de la
 * documentacion del WM IoT SDK (pinmux.rst). No editar a mano.
 */
#ifndef W80X_PINS_MUX_H
#define W80X_PINS_MUX_H

#include <stdint.h>

typedef struct {{
    uint8_t valid   : 1;  /* la pata existe en este encapsulado */
    uint8_t pwm_ch  : 4;  /* canal PWM (0xF: ninguno) */
    uint8_t pwm_fun : 4;  /* funcion del multiplexor para PWM (WM_GPIO_IOMUX_FUNx) */
    uint8_t adc_ch  : 4;  /* canal ADC (0xF: ninguno), funcion 6 */
}} w80x_pin_mux_t;

#define W80X_PIN_MUX_COUNT {n}

static const w80x_pin_mux_t w80x_pin_mux[W80X_PIN_MUX_COUNT] = {{
{body}
}};

#endif
'''


def gen_dt(soc, adc_channels):
    src = open(os.path.join(PLAT, 'sdk', soc, 'device_table', 'wm_dt_hw.c'), encoding='utf-8').read()

    def empty(array, typ, count_field, ptr_field):
        nonlocal src
        src, k = re.subn(r'const static %s %s\[\] = \{.*?\n\};\n\n' % (typ, array), '', src, flags=re.S)
        assert k == 1, array
        src = src.replace('.%s = sizeof(%s) / sizeof(%s[0])' % (count_field, array, array), '.%s = 0' % count_field)
        src = src.replace('.%s = (%s *)%s' % (ptr_field, typ, array), '.%s = NULL' % ptr_field)

    # frecuencia de la CPU: menu del IDE
    src, k = re.subn(r'\{ \.type = WM_RCC_TYPE_CPU, \.clock = \d+ \}',
                     '{ .type = WM_RCC_TYPE_CPU, .clock = W80X_CPU_MHZ }', src)
    assert k == 1
    src = src.replace('#include "wm_dt_hw.h"',
                      '#include "wm_dt_hw.h"\n\n'
                      '#ifndef F_CPU\n#define F_CPU 240000000\n#endif\n'
                      '#define W80X_CPU_MHZ (F_CPU / 1000000)', 1)
    empty('dt_hw_gpio_pin_cfg', 'wm_dt_hw_gpio_cfg_t', 'gpio_cfg_count', 'gpio_cfg')
    empty('dt_hw_pwm_pin', 'wm_dt_hw_pin_cfg_t', 'pin_cfg_count', 'pin_cfg')
    empty('dt_hw_adc_pin', 'wm_dt_hw_pin_cfg_t', 'pin_cfg_count', 'pin_cfg')
    chans = ''.join(
        '    { .adc_channel = WM_ADC_CHANNEL_%d, .pga_gain1 = WM_ADC_GAIN1_LEVEL_0, .pga_gain2 = WM_ADC_GAIN2_LEVEL_0, '
        '.adc_cmp = 0, .cmp_data = 0, .cmp_pol = 0 },\n' % c for c in adc_channels)
    src, k = re.subn(r'(const static wm_dt_hw_adc_cfg_t dt_hw_adc_adc_cfg\[\] = \{\n).*?(\};)',
                     lambda m: m.group(1) + chans + m.group(2), src, flags=re.S)
    assert k == 1
    # UART: solo TX y RX (sin RTS/CTS, que quitarian patas a otros puertos)
    def two_pins(m):
        pins = re.findall(r'    \{ \.pin_num.*?\},\n', m.group(2))
        return m.group(1) + ''.join(pins[:2]) + m.group(3)
    src = re.sub(r'(const static wm_dt_hw_pin_cfg_t dt_hw_uart\d_pin\[\] = \{\n)(.*?)(\};)', two_pins, src, flags=re.S)
    src = src.replace('.flow_ctrl = WM_UART_FLOW_CTRL_RTS_CTS', '.flow_ctrl = WM_UART_FLOW_CTRL_DISABLE')
    head = ('/*\n * Arbol de dispositivos para el nucleo Arduino (generado por\n'
            ' * tools/sdk_builder/gen_variants.py a partir del de WM IoT SDK).\n */\n')
    return head + src


PAD = ['PA%d' % i for i in range(16)] + ['PB%d' % i for i in range(32)]


def dt_pins(src, array):
    m = re.search(r'const static wm_dt_hw_pin_cfg_t %s\[\] = \{\n(.*?)\};' % array, src, re.S)
    return [int(x) for x in re.findall(r'WM_GPIO_NUM_(\d+)', m.group(1))] if m else []


def gen_pins_arduino(variant, soc, pins, dt_src):
    last = max(pins)
    enum = ', '.join(PAD[:last + 1])
    adc = sorted(n for n, (_, f) in pins.items() if any(re.fullmatch(r'ADC_\d', x) for x in f))
    if len(adc) >= 4:
        analog = ('/* como en el nucleo anterior: A1..A4 = PA1..PA4 (A0 = A1) */\n'
                  '#define A1 PA1\n#define A2 PA2\n#define A3 PA3\n#define A4 PA4\n#define A0 A1\n')
    else:
        analog = '#define A0 PA1\n#define A1 PA4\n'
    i2c = dt_pins(dt_src, 'dt_hw_i2c_pin')
    uarts = []
    for u in range(6):
        p = dt_pins(dt_src, 'dt_hw_uart%d_pin' % u)
        if p:
            uarts.append((u, p))
    ser = '\n'.join(' *   %-7s TX %-4s RX %s' % ('Serial' + (str(u) if u else ''), PAD[p[0]], PAD[p[1]])
                    for u, p in uarts)
    pwm = sorted(n for n, (_, f) in pins.items() if any(re.fullmatch(r'PWM\d', x) for x in f))
    return f'''/*
 * pins_arduino.h - patas de la placa {variant} (chip {soc.upper()}).
 * Generado por tools/sdk_builder/gen_variants.py. No editar a mano:
 * lo propio de la placa (LED, SPI...) esta en variant.h.
 *
 * Puertos serie (patas por defecto del SDK):
{ser}
 * I2C (Wire): SDA {PAD[i2c[1]]}, SCL {PAD[i2c[0]]}
 * PWM (analogWrite): {', '.join(PAD[n] for n in pwm)}
 * ADC (analogRead):  {', '.join(PAD[n] for n in adc)}
 */
#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

/* numero de pata = WM_GPIO_NUM_x del SDK */
enum pins {{
    {enum}
}};

{analog}
#define PIN_WIRE_SDA ({PAD[i2c[1]]})
#define PIN_WIRE_SCL ({PAD[i2c[0]]})
#define PIN_SERIAL_TX ({PAD[uarts[0][1][0]]})
#define PIN_SERIAL_RX ({PAD[uarts[0][1][1]]})

#define W80X_UART_COUNT {len(uarts)}
#define NUM_DIGITAL_PINS {last + 1}
#define NUM_ANALOG_INPUTS {len(adc)}
#define PINS_COUNT NUM_DIGITAL_PINS
#define ADC_COUNT NUM_ANALOG_INPUTS
#define PWM_COUNT 5

#include "variant.h"

#ifdef __cplusplus
static const uint8_t SDA = PIN_WIRE_SDA;
static const uint8_t SCL = PIN_WIRE_SCL;
#ifdef PIN_SPI_SS
static const uint8_t SS   = PIN_SPI_SS;
static const uint8_t MOSI = PIN_SPI_MOSI;
static const uint8_t MISO = PIN_SPI_MISO;
static const uint8_t SCK  = PIN_SPI_SCK;
#endif
#endif

#endif
'''


def main():
    tables = pinmux_tables()
    for variant, soc in VARIANTS.items():
        pins = tables[soc]
        adc = sorted({int(f[4:]) for _, funs in pins.values() for f in funs if re.fullmatch(r'ADC_\d', f)})
        d = os.path.join(PLAT, 'variants', variant)
        open(os.path.join(d, 'pins_mux.h'), 'w', encoding='utf-8').write(gen_pins_mux(variant, soc, pins))
        dt = gen_dt(soc, adc)
        open(os.path.join(d, 'wm_dt_hw.c'), 'w', encoding='utf-8').write(dt)
        open(os.path.join(d, 'pins_arduino.h'), 'w', encoding='utf-8').write(gen_pins_arduino(variant, soc, pins, dt))
        print('%-7s %-6s %2d patas, ADC %s' % (variant, soc, len(pins), adc))


if __name__ == '__main__':
    main()
