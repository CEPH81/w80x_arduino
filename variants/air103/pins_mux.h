/*
 * pins_mux.h - funciones de cada pata del W806 (air103)
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

#define W80X_PIN_MUX_COUNT 44

static const w80x_pin_mux_t w80x_pin_mux[W80X_PIN_MUX_COUNT] = {
    { 1, 0x2, 0x3, 0xF },    /*  0 PA0  I2S_MCLK, L-SPI_CS, PWM2, I2S_DO, GPIO */
    { 1, 0x3, 0x3, 0x0 },    /*  1 PA1  JTAG_CK, I2C_SCL, PWM3, I2S_LRCLK, GPIO, ADC_0 */
    { 1, 0x0, 0x3, 0x3 },    /*  2 PA2  UART1_RTS, UART2_TX, PWM0, UART3_RTS, GPIO, ADC_3 */
    { 1, 0x1, 0x3, 0x2 },    /*  3 PA3  UART1_CTS, UART2_RX, PWM1, UART3_CTS, GPIO, ADC_2 */
    { 1, 0x4, 0x3, 0x1 },    /*  4 PA4  JTAG_SWO, I2C_SDA, PWM4, I2S_BCLK, GPIO, ADC_1 */
    { 1, 0xF, 0xF, 0xF },    /*  5 PA5  UART3_TX, UART2_RTS, PWM_BREAK, UART4_RTS, GPIO */
    { 1, 0xF, 0xF, 0xF },    /*  6 PA6  UART3_RX, UART2_CTS, UART4_CTS, GPIO, LCD_SEG31 */
    { 1, 0x4, 0x1, 0xF },    /*  7 PA7  PWM4, L-SPI_MOSI, I2S_MCLK, I2S_DI, GPIO, LCD_SEG03 */
    { 1, 0xF, 0xF, 0xF },    /*  8 PA8  PWM_BREAK, UART4_TX, UART5_TX, I2S_BCLK, GPIO, LCD_SEG04 */
    { 1, 0xF, 0xF, 0xF },    /*  9 PA9  MMC_CLK, UART4_RX, UART5_RX, I2S_LRCLK, GPIO, LCD_SEG05 */
    { 1, 0x0, 0x3, 0xF },    /* 10 PA10 MMC_CMD, UART4_RTS, PWM0, I2S_DO, GPIO, LCD_SEG06 */
    { 1, 0x1, 0x3, 0xF },    /* 11 PA11 MMC_DAT0, UART4_CTS, PWM1, I2S_DI, GPIO, LCD_SEG07 */
    { 1, 0x2, 0x3, 0xF },    /* 12 PA12 MMC_DAT1, UART5_TX, PWM2, GPIO, LCD_SEG08 */
    { 1, 0x3, 0x3, 0xF },    /* 13 PA13 MMC_DAT2, UART5_RX, PWM3, GPIO, LCD_SEG09 */
    { 1, 0x4, 0x3, 0xF },    /* 14 PA14 MMC_DAT3, UART5_CTS, PWM4, GPIO, LCD_SEG10 */
    { 1, 0xF, 0xF, 0xF },    /* 15 PA15 PSRAM_CK, UART5_RTS, PWM_BREAK, GPIO, LCD_SEG11 */
    { 1, 0x0, 0x1, 0xF },    /* 16 PB0  PWM0, L-SPI_MISO, UART3_TX, PSRAM_CK, GPIO, LCD_SEG12 */
    { 1, 0x1, 0x1, 0xF },    /* 17 PB1  PWM1, L-SPI_CK, UART3_RX, PSRAM_CS, GPIO, LCD_SEG13 */
    { 1, 0x2, 0x1, 0xF },    /* 18 PB2  PWM2, L-SPI_CK, UART2_TX, PSRAM_DAT0, GPIO, LCD_SEG14 */
    { 1, 0x3, 0x1, 0xF },    /* 19 PB3  PWM3, L-SPI_MISO, UART2_RX, PSRAM_DAT1, GPIO, LCD_SEG15 */
    { 1, 0xF, 0xF, 0xF },    /* 20 PB4  L-SPI_CS, UART2_RTS, UART4_TX, PSRAM_DAT2, GPIO, LCD_SEG16 */
    { 1, 0xF, 0xF, 0xF },    /* 21 PB5  L-SPI_MOSI, UART2_CTS, UART4_RX, PSRAM_DAT3, GPIO, LCD_SEG17 */
    { 1, 0xF, 0xF, 0xF },    /* 22 PB6  UART1_TX, MMC_CLK, H-SPI_CK, SDIO_SLAVE_CK, GPIO, LCD_SEG18 */
    { 1, 0xF, 0xF, 0xF },    /* 23 PB7  UART1_RX, MMC_CMD, H-SPI_INT, SDIO_SLAVE_CMD, GPIO, LCD_SEG19 */
    { 1, 0xF, 0xF, 0xF },    /* 24 PB8  I2S_BCLK, MMC_DAT0, PWM_BREAK, SDIO_SLAVE_D0, GPIO, LCD_SEG20 */
    { 1, 0xF, 0xF, 0xF },    /* 25 PB9  I2S_LRCLK, MMC_DAT1, H-SPI_CS, SDIO_SLAVE_D1, GPIO, LCD_SEG21 */
    { 1, 0xF, 0xF, 0xF },    /* 26 PB10 I2S_DI, MMC_DAT2, H-SPI_DI, SDIO_SLAVE_D2, GPIO, LCD_SEG22 */
    { 1, 0xF, 0xF, 0xF },    /* 27 PB11 I2S_D0, MMC_DAT3, H-SPI_DO, SDIO_SLAVE_D3, GPIO, LCD_SEG23 */
    { 1, 0x0, 0x2, 0xF },    /* 28 PB12 H-SPI_CK, PWM0, UART5_CTS, I2S_BCLK, GPIO, LCD_SEG24 */
    { 1, 0x1, 0x2, 0xF },    /* 29 PB13 H-SPI_INT, PWM1, UART5_RTS, I2S_LRCLK, GPIO, LCD_SEG25 */
    { 1, 0x2, 0x2, 0xF },    /* 30 PB14 HSPI_CS, PWM2, L-SPI_CS, I2S_DO, GPIO, LCD_SEG26 */
    { 1, 0x3, 0x2, 0xF },    /* 31 PB15 HSPI_DI, PWM3, L-SPI_CK, I2S_DI, GPIO, LCD_SEG27 */
    { 1, 0x4, 0x2, 0xF },    /* 32 PB16 HSPI_DO, PWM4, L-SPI_MISO, UART1_RX, GPIO, LCD_SEG28 */
    { 1, 0xF, 0xF, 0xF },    /* 33 PB17 UART5_RX, PWM_BREAK, L-SPI_MOSI, I2S_MCLK, GPIO, LCD_SEG29 */
    { 1, 0xF, 0xF, 0xF },    /* 34 PB18 UART5_TX, GPIO, LCD_SEG30 */
    { 1, 0x0, 0x2, 0xF },    /* 35 PB19 UART0_TX, PWM0, UART1_RTS, I2C_SDA, GPIO */
    { 1, 0x1, 0x2, 0xF },    /* 36 PB20 UART0_RX, PWM1, UART1_CTS, I2C_SCL, GPIO */
    { 1, 0xF, 0xF, 0xF },    /* 37 PB21 UART0_RTS, GPIO, LCD_COM1 */
    { 1, 0xF, 0xF, 0xF },    /* 38 PB22 UART0_CTS, GPIO, LCD_COM2 */
    { 0, 0, 0, 0 },          /* 39: no existe */
    { 1, 0x2, 0x2, 0xF },    /* 40 PB24 L-SPI_CK, PWM2, GPIO, LCD_SEG2 */
    { 1, 0x3, 0x2, 0xF },    /* 41 PB25 L-SPI_MISO, PWM3, GPIO, LCD_COM0 */
    { 1, 0x4, 0x2, 0xF },    /* 42 PB26 L-SPI_MOSI, PWM4, GPIO, LCD_SEG1 */
    { 1, 0xF, 0xF, 0xF },    /* 43 PB27 PSRAM_CS, UART0_TX, GPIO, LCD_COM3 */
};

#endif
