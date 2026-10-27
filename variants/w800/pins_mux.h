/*
 * pins_mux.h - funciones de cada pata del W800 (w800)
 * Generado por tools/sdk_builder/gen_variants.py a partir de la
 * documentacion del WM IoT SDK (pinmux.rst). No editar a mano.
 */
#ifndef W80X_PINS_MUX_H
#define W80X_PINS_MUX_H

#include <stdint.h>

typedef struct {
    uint8_t valid   : 1;  /* la pata existe en este encapsulado */
    uint8_t pwm_ch  : 4;  /* canal PWM (0xF: ninguno) */
    uint8_t pwm_fun : 4;  /* funcion del multiplexor para PWM (WM_GPIO_IOMUX_FUNx) */
    uint8_t adc_ch  : 4;  /* canal ADC (0xF: ninguno), funcion 6 */
} w80x_pin_mux_t;

#define W80X_PIN_MUX_COUNT 37

static const w80x_pin_mux_t w80x_pin_mux[W80X_PIN_MUX_COUNT] = {
    { 1, 0x2, 0x3, 0xF },    /*  0 PA0  I2S_MCLK, L-SPI_CS, PWM2, I2S_DO, GPIO */
    { 1, 0x3, 0x3, 0x0 },    /*  1 PA1  JTAG_CK, I2C_SCL, PWM3, I2S_LRCLK, GPIO, ADC_0 */
    { 0, 0, 0, 0 },          /*  2: no existe */
    { 0, 0, 0, 0 },          /*  3: no existe */
    { 1, 0x4, 0x3, 0x1 },    /*  4 PA4  JTAG_SWO, I2C_SDA, PWM4, I2S_BCLK, GPIO, ADC_1 */
    { 0, 0, 0, 0 },          /*  5: no existe */
    { 0, 0, 0, 0 },          /*  6: no existe */
    { 1, 0x4, 0x1, 0xF },    /*  7 PA7  PWM4, L-SPI_MOSI, I2S_MCLK, I2S_DI, GPIO */
    { 0, 0, 0, 0 },          /*  8: no existe */
    { 0, 0, 0, 0 },          /*  9: no existe */
    { 0, 0, 0, 0 },          /* 10: no existe */
    { 0, 0, 0, 0 },          /* 11: no existe */
    { 0, 0, 0, 0 },          /* 12: no existe */
    { 0, 0, 0, 0 },          /* 13: no existe */
    { 0, 0, 0, 0 },          /* 14: no existe */
    { 0, 0, 0, 0 },          /* 15: no existe */
    { 1, 0x0, 0x1, 0xF },    /* 16 PB0  PWM0, L-SPI_MISO, UART3_TX, PSRAM_CK, GPIO */
    { 1, 0x1, 0x1, 0xF },    /* 17 PB1  PWM1, L-SPI_CK, UART3_RX, PSRAM_CS, GPIO */
    { 1, 0x2, 0x1, 0xF },    /* 18 PB2  PWM2, L-SPI_CK, UART2_TX, PSRAM_DAT0, GPIO */
    { 1, 0x3, 0x1, 0xF },    /* 19 PB3  PWM3, L-SPI_MISO, UART2_RX, PSRAM_DAT1, GPIO */
    { 1, 0xF, 0xF, 0xF },    /* 20 PB4  L-SPI_CS, UART2_RTS, UART4_TX, PSRAM_DAT2, GPIO */
    { 1, 0xF, 0xF, 0xF },    /* 21 PB5  L-SPI_MOSI, UART2_CTS, UART4_RX, PSRAM_DAT3, GPIO */
    { 1, 0xF, 0xF, 0xF },    /* 22 PB6  UART1_TX, MMC_CLK, H-SPI_CK, SDIO_SLAVE_CK, GPIO */
    { 1, 0xF, 0xF, 0xF },    /* 23 PB7  UART1_RX, MMC_CMD, H-SPI_INT, SDIO_SLAVE_CMD, GPIO */
    { 1, 0xF, 0xF, 0xF },    /* 24 PB8  I2S_BCLK, MMC_DAT0, PWM_BREAK, SDIO_SLAVE_D0, GPIO */
    { 1, 0xF, 0xF, 0xF },    /* 25 PB9  I2S_LRCLK, MMC_DAT1, H-SPI_CS, SDIO_SLAVE_D1, GPIO */
    { 1, 0xF, 0xF, 0xF },    /* 26 PB10 I2S_DI, MMC_DAT2, H-SPI_DI, SDIO_SLAVE_D2, GPIO */
    { 1, 0xF, 0xF, 0xF },    /* 27 PB11 I2S_D0, MMC_DAT3, H-SPI_DO, SDIO_SLAVE_D3, GPIO */
    { 0, 0, 0, 0 },          /* 28: no existe */
    { 0, 0, 0, 0 },          /* 29: no existe */
    { 0, 0, 0, 0 },          /* 30: no existe */
    { 0, 0, 0, 0 },          /* 31: no existe */
    { 0, 0, 0, 0 },          /* 32: no existe */
    { 0, 0, 0, 0 },          /* 33: no existe */
    { 0, 0, 0, 0 },          /* 34: no existe */
    { 1, 0x0, 0x2, 0xF },    /* 35 PB19 UART0_TX, PWM0, UART1_RTS, I2C_SDA, GPIO */
    { 1, 0x1, 0x2, 0xF },    /* 36 PB20 UART0_RX, PWM1, UART1_CTS, I2C_SCL, GPIO */
};

#endif
