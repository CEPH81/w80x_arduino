#ifndef _VARIANT_ARDUINO_W802_
#define _VARIANT_ARDUINO_W802_

// TFT LCD CONNECTOR
#define PIN_TFT_RST	(PA7)
#define PIN_TFT_RS	(PA12)
#define PIN_TFT_CS	(PA14)
#define PIN_TFT_SCL	(PA9)
#define PIN_TFT_SDI	(PA10)
#define PIN_TFT_LED	(PB8)
#define PIN_TFT_TE	(PB29)

// La placa W802 no trae LED de usuario: LED_BUILTIN es la luz del conector TFT (PB8).
// Cambie estas lineas si conecta LEDs en otras patas.
#define LED_BUILTIN_1 PIN_TFT_LED
#define LED_BUILTIN_2 PB9
#define LED_BUILTIN_3 PB10
#define LED_BUILTIN   LED_BUILTIN_1

// SPI
#define PIN_SPI_SS    (PB14) 
#define PIN_SPI_SCK   (PB15)
#define PIN_SPI_MISO  (PB16)
#define PIN_SPI_MOSI  (PB17)


#endif /* _VARIANT_ARDUINO_W802_ */
