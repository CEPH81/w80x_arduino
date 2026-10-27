/*
 * Arbol de dispositivos para el nucleo Arduino (generado por
 * tools/sdk_builder/gen_variants.py a partir del de WM IoT SDK).
 */
/**
 * @file wm_dt_hw.c
 *
 * @brief Device Hardware Information Module
 *
 */

/**
 *  Copyright 2022-2024 Beijing WinnerMicroelectronics Co.,Ltd.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "wm_dt_hw.h"

#ifndef F_CPU
#define F_CPU 240000000
#endif
#define W80X_CPU_MHZ (F_CPU / 1000000)
#include "wm_dt_op.h"

#include "wm_soc_cfgs.h"
#include "wm_irq.h"


struct wm_drv_ops_structure;

typedef struct wm_drv_ops_structure wm_drv_rcc_ops_t;
extern const wm_drv_rcc_ops_t wm_drv_rcc_ops;

typedef struct wm_drv_ops_structure wm_drv_dma_ops_t;
extern const wm_drv_dma_ops_t wm_drv_dma_ops;

typedef struct wm_drv_ops_structure wm_drv_gpio_ops_t;
extern const wm_drv_gpio_ops_t wm_drv_gpio_ops;

typedef struct wm_drv_ops_structure wm_drv_timer_ops_t;
extern const wm_drv_timer_ops_t wm_drv_timer_ops;

typedef struct wm_drv_ops_structure wm_drv_uart_ops_t;
extern const wm_drv_uart_ops_t wm_drv_uart_ops;

typedef struct wm_drv_ops_structure wm_drv_pwm_ops_t;
extern const wm_drv_pwm_ops_t wm_drv_pwm_ops;

typedef struct wm_drv_ops_structure wm_drv_iflash_ops_t;
extern const wm_drv_iflash_ops_t wm_drv_internal_flash_ops;

typedef struct wm_drv_ops_structure wm_drv_i2c_ops_t;
extern const wm_drv_i2c_ops_t wm_drv_i2c_ops;

typedef struct wm_drv_ops_structure wm_drv_eeprom_ops_t;
extern const wm_drv_eeprom_ops_t wm_drv_eeprom_nv24c0x_ops;

typedef struct wm_drv_ops_structure wm_drv_spim_ops_t;
extern const wm_drv_spim_ops_t wm_drv_spim_ops;

typedef struct wm_drv_ops_structure wm_drv_rtc_ops_t;
extern const wm_drv_rtc_ops_t wm_drv_rtc_ops;

typedef struct wm_drv_ops_structure wm_drv_pmu_ops_t;
extern const wm_drv_pmu_ops_t wm_drv_pmu_ops;

typedef struct wm_drv_ops_structure wm_drv_adc_ops_t;
extern const wm_drv_adc_ops_t wm_drv_adc_ops;

typedef struct wm_drv_ops_structure wm_drv_i2s_ops_t;
extern const wm_drv_i2s_ops_t wm_drv_i2s_ops;

typedef struct wm_drv_ops_structure wm_drv_wdt_ops_t;
extern const wm_drv_wdt_ops_t wm_drv_wdt_ops;

typedef struct wm_drv_ops_structure wm_drv_crc_ops_t;
extern const wm_drv_crc_ops_t wm_drv_crc_ops;

typedef struct wm_drv_ops_structure wm_drv_crypto_ops_t;
extern const wm_drv_crypto_ops_t wm_drv_crypto_ops;

typedef struct wm_drv_ops_structure wm_drv_hash_ops_t;
extern const wm_drv_hash_ops_t wm_drv_hash_ops;

typedef struct wm_drv_ops_structure wm_drv_rng_ops_t;
extern const wm_drv_rng_ops_t wm_drv_rng_ops;

typedef struct wm_drv_ops_structure wm_drv_rsa_ops_t;
extern const wm_drv_rsa_ops_t wm_drv_rsa_ops;

const static wm_dt_hw_rcc_cfg_t dt_hw_rcc_rcc[] = {
    { .type = WM_RCC_TYPE_CPU, .clock = W80X_CPU_MHZ },
    { .type = WM_RCC_TYPE_WLAN, .clock = 160 },
};

const static wm_dt_hw_rcc_t dt_hw_rcc = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40000e00,
    .rcc_cfg_count = sizeof(dt_hw_rcc_rcc) / sizeof(dt_hw_rcc_rcc[0]),
    .rcc_cfg = (wm_dt_hw_rcc_cfg_t *)dt_hw_rcc_rcc,
};

const static wm_dt_hw_dma_t dt_hw_dma = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40000800,
    .irq_cfg = { .irq_num = WM_IRQ_DMA_Channel0, .irq_priority = 0 },
};

const static wm_dt_hw_gpio_t dt_hw_gpio = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .gpio_cfg_count = 0,
    .gpio_cfg = NULL,
};

const static wm_dt_hw_timer_t dt_hw_timer0 = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40011800,
    .irq_cfg = { .irq_num = WM_IRQ_TIMER, .irq_priority = 0 },
};

const static wm_dt_hw_timer_t dt_hw_timer1 = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40011800,
    .irq_cfg = { .irq_num = WM_IRQ_TIMER, .irq_priority = 0 },
};

const static wm_dt_hw_timer_t dt_hw_timer2 = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40011800,
    .irq_cfg = { .irq_num = WM_IRQ_TIMER, .irq_priority = 0 },
};

const static wm_dt_hw_timer_t dt_hw_timer3 = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40011800,
    .irq_cfg = { .irq_num = WM_IRQ_TIMER, .irq_priority = 0 },
};

const static wm_dt_hw_timer_t dt_hw_timer4 = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40011800,
    .irq_cfg = { .irq_num = WM_IRQ_TIMER, .irq_priority = 0 },
};

const static wm_dt_hw_timer_t dt_hw_timer5 = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40011800,
    .irq_cfg = { .irq_num = WM_IRQ_TIMER, .irq_priority = 0 },
};

const static wm_dt_hw_pin_cfg_t dt_hw_uart0_pin[] = {
    { .pin_num = WM_GPIO_NUM_35, .pin_mux = WM_GPIO_IOMUX_FUN1 },
    { .pin_num = WM_GPIO_NUM_36, .pin_mux = WM_GPIO_IOMUX_FUN1 },
};

const static wm_dt_hw_uart_t dt_hw_uart0 = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40010600,
    .irq_cfg = { .irq_num = WM_IRQ_UART0, .irq_priority = 0 },
    .uart_cfg = { .baudrate = WM_UART_BAUDRATE_B115200, .parity = WM_UART_PARITY_NONE, .stop_bits = WM_UART_STOP_BIT_1, .data_bits = WM_UART_DATA_BIT_8, .flow_ctrl = WM_UART_FLOW_CTRL_DISABLE },
    .pin_cfg_count = sizeof(dt_hw_uart0_pin) / sizeof(dt_hw_uart0_pin[0]),
    .pin_cfg = (wm_dt_hw_pin_cfg_t *)dt_hw_uart0_pin,
    .rcc_device_name = "rcc",
};

const static wm_dt_hw_pin_cfg_t dt_hw_uart1_pin[] = {
    { .pin_num = WM_GPIO_NUM_22, .pin_mux = WM_GPIO_IOMUX_FUN1 },
    { .pin_num = WM_GPIO_NUM_23, .pin_mux = WM_GPIO_IOMUX_FUN1 },
};

const static wm_dt_hw_uart_t dt_hw_uart1 = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40010800,
    .irq_cfg = { .irq_num = WM_IRQ_UART1, .irq_priority = 0 },
    .uart_cfg = { .baudrate = WM_UART_BAUDRATE_B115200, .parity = WM_UART_PARITY_NONE, .stop_bits = WM_UART_STOP_BIT_1, .data_bits = WM_UART_DATA_BIT_8, .flow_ctrl = WM_UART_FLOW_CTRL_DISABLE },
    .pin_cfg_count = sizeof(dt_hw_uart1_pin) / sizeof(dt_hw_uart1_pin[0]),
    .pin_cfg = (wm_dt_hw_pin_cfg_t *)dt_hw_uart1_pin,
    .rcc_device_name = "rcc",
};

const static wm_dt_hw_pin_cfg_t dt_hw_uart2_pin[] = {
    { .pin_num = WM_GPIO_NUM_18, .pin_mux = WM_GPIO_IOMUX_FUN3 },
    { .pin_num = WM_GPIO_NUM_19, .pin_mux = WM_GPIO_IOMUX_FUN3 },
};

const static wm_dt_hw_uart_t dt_hw_uart2 = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40010a00,
    .irq_cfg = { .irq_num = WM_IRQ_UART24, .irq_priority = 0 },
    .uart_cfg = { .baudrate = WM_UART_BAUDRATE_B115200, .parity = WM_UART_PARITY_NONE, .stop_bits = WM_UART_STOP_BIT_1, .data_bits = WM_UART_DATA_BIT_8, .flow_ctrl = WM_UART_FLOW_CTRL_DISABLE },
    .pin_cfg_count = sizeof(dt_hw_uart2_pin) / sizeof(dt_hw_uart2_pin[0]),
    .pin_cfg = (wm_dt_hw_pin_cfg_t *)dt_hw_uart2_pin,
    .rcc_device_name = "rcc",
};

const static wm_dt_hw_pin_cfg_t dt_hw_uart3_pin[] = {
    { .pin_num = WM_GPIO_NUM_16, .pin_mux = WM_GPIO_IOMUX_FUN3 },
    { .pin_num = WM_GPIO_NUM_17, .pin_mux = WM_GPIO_IOMUX_FUN3 },
};

const static wm_dt_hw_uart_t dt_hw_uart3 = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40010c00,
    .irq_cfg = { .irq_num = WM_IRQ_UART24, .irq_priority = 0 },
    .uart_cfg = { .baudrate = WM_UART_BAUDRATE_B115200, .parity = WM_UART_PARITY_NONE, .stop_bits = WM_UART_STOP_BIT_1, .data_bits = WM_UART_DATA_BIT_8, .flow_ctrl = WM_UART_FLOW_CTRL_DISABLE },
    .pin_cfg_count = sizeof(dt_hw_uart3_pin) / sizeof(dt_hw_uart3_pin[0]),
    .pin_cfg = (wm_dt_hw_pin_cfg_t *)dt_hw_uart3_pin,
    .dma_device_name = "dma",
    .rcc_device_name = "rcc",
};

const static wm_dt_hw_pin_cfg_t dt_hw_uart4_pin[] = {
    { .pin_num = WM_GPIO_NUM_20, .pin_mux = WM_GPIO_IOMUX_FUN3 },
    { .pin_num = WM_GPIO_NUM_21, .pin_mux = WM_GPIO_IOMUX_FUN3 },
};

const static wm_dt_hw_uart_t dt_hw_uart4 = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40010e00,
    .irq_cfg = { .irq_num = WM_IRQ_UART24, .irq_priority = 0 },
    .uart_cfg = { .baudrate = WM_UART_BAUDRATE_B115200, .parity = WM_UART_PARITY_NONE, .stop_bits = WM_UART_STOP_BIT_1, .data_bits = WM_UART_DATA_BIT_8, .flow_ctrl = WM_UART_FLOW_CTRL_DISABLE },
    .pin_cfg_count = sizeof(dt_hw_uart4_pin) / sizeof(dt_hw_uart4_pin[0]),
    .pin_cfg = (wm_dt_hw_pin_cfg_t *)dt_hw_uart4_pin,
    .dma_device_name = "dma",
    .rcc_device_name = "rcc",
};

const static wm_dt_hw_pin_cfg_t dt_hw_uart5_pin[] = {
    { .pin_num = WM_GPIO_NUM_34, .pin_mux = WM_GPIO_IOMUX_FUN3 },
    { .pin_num = WM_GPIO_NUM_33, .pin_mux = WM_GPIO_IOMUX_FUN3 },
};

const static wm_dt_hw_uart_t dt_hw_uart5 = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40011000,
    .irq_cfg = { .irq_num = WM_IRQ_UART24, .irq_priority = 0 },
    .uart_cfg = { .baudrate = WM_UART_BAUDRATE_B115200, .parity = WM_UART_PARITY_NONE, .stop_bits = WM_UART_STOP_BIT_1, .data_bits = WM_UART_DATA_BIT_8, .flow_ctrl = WM_UART_FLOW_CTRL_DISABLE },
    .pin_cfg_count = sizeof(dt_hw_uart5_pin) / sizeof(dt_hw_uart5_pin[0]),
    .pin_cfg = (wm_dt_hw_pin_cfg_t *)dt_hw_uart5_pin,
    .rcc_device_name = "rcc",
};

const static wm_dt_hw_pwm_t dt_hw_pwm = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40011e00,
    .irq_cfg = { .irq_num = WM_IRQ_PWM, .irq_priority = 0 },
    .pin_cfg_count = 0,
    .pin_cfg = NULL,
    .dma_device_name = "dma",
};

const static wm_dt_hw_iflash_t dt_hw_iflash = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40002000,
    .flash_cfg = { .quad_spi = true },
};

const static wm_dt_hw_pin_cfg_t dt_hw_i2c_pin[] = {
    { .pin_num = WM_GPIO_NUM_1, .pin_mux = WM_GPIO_IOMUX_FUN2 },
    { .pin_num = WM_GPIO_NUM_4, .pin_mux = WM_GPIO_IOMUX_FUN2 },
};

const static wm_dt_hw_i2c_t dt_hw_i2c = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40010000,
    .irq_cfg = { .irq_num = WM_IRQ_I2C, .irq_priority = 0 },
    .i2c_cfg = { .max_clock = 400000, .addr_10_bits = false },
    .pin_cfg_count = sizeof(dt_hw_i2c_pin) / sizeof(dt_hw_i2c_pin[0]),
    .pin_cfg = (wm_dt_hw_pin_cfg_t *)dt_hw_i2c_pin,
    .rcc_device_name = "rcc",
};

const static wm_dt_hw_eeprom_t dt_hw_eeprom0 = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .speed_hz = 400000,
    .i2c_addr = 0x52,
    .size = 256,
    .page_size = 16,
    .addr_width = 8,
    .read_only = false,
    .max_write_time_ms = 5,
    .i2c_device_name = "i2c",
};

const static wm_dt_hw_pin_cfg_t dt_hw_eeprom1_pin[] = {
    { .pin_num = WM_GPIO_NUM_21, .pin_mux = WM_GPIO_IOMUX_FUN5, .pin_dir = WM_GPIO_DIR_OUTPUT, .pin_pupd = WM_GPIO_FLOAT },
};

const static wm_dt_hw_eeprom_t dt_hw_eeprom1 = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .speed_hz = 100000,
    .i2c_addr = 0x50,
    .size = 256,
    .page_size = 16,
    .addr_width = 8,
    .read_only = false,
    .max_write_time_ms = 10,
    .i2c_device_name = "i2c",
    .pin_cfg_count = sizeof(dt_hw_eeprom1_pin) / sizeof(dt_hw_eeprom1_pin[0]),
    .pin_cfg = (wm_dt_hw_pin_cfg_t *)dt_hw_eeprom1_pin,
};

const static wm_dt_hw_pin_cfg_t dt_hw_spim_pin[] = {
    { .pin_num = WM_GPIO_NUM_17, .pin_mux = WM_GPIO_IOMUX_FUN2 },
    { .pin_num = WM_GPIO_NUM_16, .pin_mux = WM_GPIO_IOMUX_FUN2 },
    { .pin_num = WM_GPIO_NUM_7, .pin_mux = WM_GPIO_IOMUX_FUN2 },
};

const static wm_dt_hw_spim_t dt_hw_spim = {
    .reg_base = 0x40010400,
    .irq_cfg = { .irq_num = WM_IRQ_SPI_LS, .irq_priority = 0 },
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .dma_device_name = "dma",
    .rcc_device_name = "rcc",
    .pin_cfg_count = sizeof(dt_hw_spim_pin) / sizeof(dt_hw_spim_pin[0]),
    .pin_cfg = (wm_dt_hw_pin_cfg_t *)dt_hw_spim_pin,
};

const static wm_dt_hw_rtc_t dt_hw_rtc = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40000d0c,
    .irq_cfg = { .irq_num = WM_IRQ_PMU, .irq_priority = 0 },
};

const static wm_dt_hw_pmu_t dt_hw_pmu = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40000d00,
    .irq_cfg = { .irq_num = WM_IRQ_PMU, .irq_priority = 0 },
    .clk_src = WM_PMU_CLOCK_SRC_32K,
};

const static wm_dt_hw_adc_cfg_t dt_hw_adc_adc_cfg[] = {
    { .adc_channel = WM_ADC_CHANNEL_0, .pga_gain1 = WM_ADC_GAIN1_LEVEL_0, .pga_gain2 = WM_ADC_GAIN2_LEVEL_0, .adc_cmp = 0, .cmp_data = 0, .cmp_pol = 0 },
    { .adc_channel = WM_ADC_CHANNEL_1, .pga_gain1 = WM_ADC_GAIN1_LEVEL_0, .pga_gain2 = WM_ADC_GAIN2_LEVEL_0, .adc_cmp = 0, .cmp_data = 0, .cmp_pol = 0 },
    { .adc_channel = WM_ADC_CHANNEL_2, .pga_gain1 = WM_ADC_GAIN1_LEVEL_0, .pga_gain2 = WM_ADC_GAIN2_LEVEL_0, .adc_cmp = 0, .cmp_data = 0, .cmp_pol = 0 },
    { .adc_channel = WM_ADC_CHANNEL_3, .pga_gain1 = WM_ADC_GAIN1_LEVEL_0, .pga_gain2 = WM_ADC_GAIN2_LEVEL_0, .adc_cmp = 0, .cmp_data = 0, .cmp_pol = 0 },
};

const static wm_dt_hw_adc_t dt_hw_adc = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40010200,
    .irq_cfg = { .irq_num = WM_IRQ_ADC, .irq_priority = 0 },
    .pin_cfg_count = 0,
    .pin_cfg = NULL,
    .adc_cfg_count = sizeof(dt_hw_adc_adc_cfg) / sizeof(dt_hw_adc_adc_cfg[0]),
    .adc_cfg = (wm_dt_hw_adc_cfg_t *)dt_hw_adc_adc_cfg,
    .dma_device_name = "dma",
    .rcc_device_name = "rcc",
};

const static wm_dt_hw_pin_cfg_t dt_hw_i2s_pin[] = {
    { .pin_num = WM_GPIO_NUM_24, .pin_mux = WM_GPIO_IOMUX_FUN1 },
    { .pin_num = WM_GPIO_NUM_25, .pin_mux = WM_GPIO_IOMUX_FUN1 },
    { .pin_num = WM_GPIO_NUM_7, .pin_mux = WM_GPIO_IOMUX_FUN3 },
    { .pin_num = WM_GPIO_NUM_26, .pin_mux = WM_GPIO_IOMUX_FUN1 },
    { .pin_num = WM_GPIO_NUM_27, .pin_mux = WM_GPIO_IOMUX_FUN1 },
};

const static wm_dt_hw_i2s_t dt_hw_i2s = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40012000,
    .irq_cfg = { .irq_num = WM_IRQ_I2S, .irq_priority = 0 },
    .i2s_cfg = { .extal_clock_en = false, .mclk_hz = 80000000 },
    .pin_cfg_count = sizeof(dt_hw_i2s_pin) / sizeof(dt_hw_i2s_pin[0]),
    .pin_cfg = (wm_dt_hw_pin_cfg_t *)dt_hw_i2s_pin,
    .dma_device_name = "dma",
    .rcc_device_name = "rcc",
};

const static wm_dt_hw_wdt_t dt_hw_wdt = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40011600,
    .irq_cfg = { .irq_num = WM_IRQ_WDG, .irq_priority = 0 },
    .wdt_cfg = { .counter_value = 10000000 },
};

const static wm_dt_hw_crc_t dt_hw_crc = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40000600,
    .rcc_device_name = "rcc",
};

const static wm_dt_hw_crypto_t dt_hw_crypto = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40000600,
    .rcc_device_name = "rcc",
};

const static wm_dt_hw_hash_t dt_hw_hash = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40000600,
    .rcc_device_name = "rcc",
};

const static wm_dt_hw_rng_t dt_hw_rng = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40000600,
    .rcc_device_name = "rcc",
};

const static wm_dt_hw_rsa_t dt_hw_rsa = {
    .init_cfg = { .init_level = 0, .init_priority = 0 },
    .reg_base = 0x40000000,
    .rcc_device_name = "rcc",
};

const static struct wm_dt_table_entry dt_hw_table_entry[] = {
    { .dev_name = "rcc", .hw_addr = (void *)&dt_hw_rcc, .ops_addr = (void *)&wm_drv_rcc_ops },
    { .dev_name = "dma", .hw_addr = (void *)&dt_hw_dma, .ops_addr = (void *)&wm_drv_dma_ops },
    { .dev_name = "gpio", .hw_addr = (void *)&dt_hw_gpio, .ops_addr = (void *)&wm_drv_gpio_ops },
    { .dev_name = "timer0", .hw_addr = (void *)&dt_hw_timer0, .ops_addr = (void *)&wm_drv_timer_ops },
    { .dev_name = "timer1", .hw_addr = (void *)&dt_hw_timer1, .ops_addr = (void *)&wm_drv_timer_ops },
    { .dev_name = "timer2", .hw_addr = (void *)&dt_hw_timer2, .ops_addr = (void *)&wm_drv_timer_ops },
    { .dev_name = "timer3", .hw_addr = (void *)&dt_hw_timer3, .ops_addr = (void *)&wm_drv_timer_ops },
    { .dev_name = "timer4", .hw_addr = (void *)&dt_hw_timer4, .ops_addr = (void *)&wm_drv_timer_ops },
    { .dev_name = "timer5", .hw_addr = (void *)&dt_hw_timer5, .ops_addr = (void *)&wm_drv_timer_ops },
    { .dev_name = "uart0", .hw_addr = (void *)&dt_hw_uart0, .ops_addr = (void *)&wm_drv_uart_ops },
    { .dev_name = "uart1", .hw_addr = (void *)&dt_hw_uart1, .ops_addr = (void *)&wm_drv_uart_ops },
    { .dev_name = "uart2", .hw_addr = (void *)&dt_hw_uart2, .ops_addr = (void *)&wm_drv_uart_ops },
    { .dev_name = "uart3", .hw_addr = (void *)&dt_hw_uart3, .ops_addr = (void *)&wm_drv_uart_ops },
    { .dev_name = "uart4", .hw_addr = (void *)&dt_hw_uart4, .ops_addr = (void *)&wm_drv_uart_ops },
    { .dev_name = "uart5", .hw_addr = (void *)&dt_hw_uart5, .ops_addr = (void *)&wm_drv_uart_ops },
    { .dev_name = "pwm", .hw_addr = (void *)&dt_hw_pwm, .ops_addr = (void *)&wm_drv_pwm_ops },
    { .dev_name = "iflash", .hw_addr = (void *)&dt_hw_iflash, .ops_addr = (void *)&wm_drv_internal_flash_ops },
    { .dev_name = "i2c", .hw_addr = (void *)&dt_hw_i2c, .ops_addr = (void *)&wm_drv_i2c_ops },
    { .dev_name = "eeprom0", .hw_addr = (void *)&dt_hw_eeprom0, .ops_addr = (void *)&wm_drv_eeprom_nv24c0x_ops },
    { .dev_name = "eeprom1", .hw_addr = (void *)&dt_hw_eeprom1, .ops_addr = (void *)&wm_drv_eeprom_nv24c0x_ops },
    { .dev_name = "spim", .hw_addr = (void *)&dt_hw_spim, .ops_addr = (void *)&wm_drv_spim_ops },
    { .dev_name = "rtc", .hw_addr = (void *)&dt_hw_rtc, .ops_addr = (void *)&wm_drv_rtc_ops },
    { .dev_name = "pmu", .hw_addr = (void *)&dt_hw_pmu, .ops_addr = (void *)&wm_drv_pmu_ops },
    { .dev_name = "adc", .hw_addr = (void *)&dt_hw_adc, .ops_addr = (void *)&wm_drv_adc_ops },
    { .dev_name = "i2s", .hw_addr = (void *)&dt_hw_i2s, .ops_addr = (void *)&wm_drv_i2s_ops },
    { .dev_name = "wdt", .hw_addr = (void *)&dt_hw_wdt, .ops_addr = (void *)&wm_drv_wdt_ops },
    { .dev_name = "crc", .hw_addr = (void *)&dt_hw_crc, .ops_addr = (void *)&wm_drv_crc_ops },
    { .dev_name = "crypto", .hw_addr = (void *)&dt_hw_crypto, .ops_addr = (void *)&wm_drv_crypto_ops },
    { .dev_name = "hash", .hw_addr = (void *)&dt_hw_hash, .ops_addr = (void *)&wm_drv_hash_ops },
    { .dev_name = "rng", .hw_addr = (void *)&dt_hw_rng, .ops_addr = (void *)&wm_drv_rng_ops },
    { .dev_name = "rsa", .hw_addr = (void *)&dt_hw_rsa, .ops_addr = (void *)&wm_drv_rsa_ops },
};

WM_DT_TABLE_DEFINE(, (sizeof(dt_hw_table_entry) / sizeof(dt_hw_table_entry[0])), (void *)&dt_hw_table_entry[0]);