/*
 * pins_arduino.h - patas de la placa w800 (chip W800).
 * Generado por tools/sdk_builder/gen_variants.py. No editar a mano:
 * lo propio de la placa (LED, SPI...) esta en variant.h.
 *
 * Puertos serie (patas por defecto del SDK):
 *   Serial  TX PB19 RX PB20
 *   Serial1 TX PB6  RX PB7
 *   Serial2 TX PB2  RX PB3
 *   Serial3 TX PB0  RX PB1
 *   Serial4 TX PB4  RX PB5
 * I2C (Wire): SDA PA4, SCL PA1
 * PWM (analogWrite): PA0, PA1, PA4, PA7, PB0, PB1, PB2, PB3, PB19, PB20
 * ADC (analogRead):  PA1, PA4
 */
#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

/* numero de pata = WM_GPIO_NUM_x del SDK */
enum pins {
    PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7, PA8, PA9, PA10, PA11, PA12, PA13, PA14, PA15, PB0, PB1, PB2, PB3, PB4, PB5, PB6, PB7, PB8, PB9, PB10, PB11, PB12, PB13, PB14, PB15, PB16, PB17, PB18, PB19, PB20
};

#define A0 PA1
#define A1 PA4

#define PIN_WIRE_SDA (PA4)
#define PIN_WIRE_SCL (PA1)
#define PIN_SERIAL_TX (PB19)
#define PIN_SERIAL_RX (PB20)

#define W80X_UART_COUNT 5
#define NUM_DIGITAL_PINS 37
#define NUM_ANALOG_INPUTS 2
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
