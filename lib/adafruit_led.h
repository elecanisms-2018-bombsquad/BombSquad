/*
Code ported from Adafruit_LEDBackpack:

***************************************************
This is a library for our I2C LED Backpacks

Designed specifically to work with the Adafruit LED Matrix backpacks
----> http://www.adafruit.com/products/
----> http://www.adafruit.com/products/

These displays use I2C to communicate, 2 pins are required to
interface. There are multiple selectable I2C addresses. For backpacks
with 2 Address Select pins: 0x70, 0x71, 0x72 or 0x73. For backpacks
with 3 Address Select pins: 0x70 thru 0x77

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.
MIT license, all text above must be included in any redistribution
*/


#ifndef _ADAFRUIT_LED_
#define _ADAFRUIT_LED_

#include "i2c_reg.h"

#define LED_ON 1
#define LED_OFF 0

#define LED_RED 1
#define LED_YELLOW 2
#define LED_GREEN 3



#define HT16K33_BLINK_CMD 0x80
#define HT16K33_BLINK_DISPLAYON 0x01
#define HT16K33_BLINK_OFF 0
#define HT16K33_BLINK_2HZ  1
#define HT16K33_BLINK_1HZ  2
#define HT16K33_BLINK_HALFHZ  3

#define HT16K33_CMD_BRIGHTNESS 0xE0

#define SEVENSEG_DIGITS 5


#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2
#define BYTE 0


/*
- Struct definition
- initializer function (basically a constructor?) with an i2c address and struct pointer
*/
// TODO _ADAFRUIT_LED
typedef struct {
    uint16_t displaybuffer[8];
    uint8_t  i2c_addr;
} _ADAFRUIT_LED;

void led_begin(_ADAFRUIT_LED* ptr, uint8_t addr);
void led_setBrightness(_ADAFRUIT_LED* ptr, uint8_t b);
void led_blinkRate(_ADAFRUIT_LED* ptr, uint8_t b);
void led_writeDisplay(_ADAFRUIT_LED* ptr);
void led_clear(_ADAFRUIT_LED* ptr);

//TODO alphanum ****************************************************************

typedef struct {
    _ADAFRUIT_LED super;
} _ALPHANUM;

void alphanum_begin(_ALPHANUM* ptr, uint8_t addr);
void alphanum_writeDigitRaw(_ALPHANUM* ptr, uint8_t n, uint16_t bitmask);
void alphanum_writeDigitAscii(_ALPHANUM* ptr, uint8_t n, uint8_t ascii, uint8_t dot);

//TODO 24bargraph **************************************************************
typedef struct {
    _ADAFRUIT_LED super;
} _BARGRAPH;

void bargraph_begin(_BARGRAPH* ptr, uint8_t addr);
void bargraph_setBar(_BARGRAPH* ptr, uint8_t bar, uint8_t color);

//TODO 7segment ****************************************************************
typedef struct {
    _ADAFRUIT_LED super;
    uint8_t  position;
} _7SEGMENT;

uint8_t write(_7SEGMENT* ptr, uint8_t c);

/* I don't think we need this
void sevseg_print_c(_7SEGMENT* ptr, char c);
void sevseg_print_uc(_7SEGMENT* ptr, unsigned char uc);
void sevseg_print_i(_7SEGMENT* ptr, int i);
void sevseg_print_ui(_7SEGMENT* ptr, unsigned int ui);
void sevseg_print_l(_7SEGMENT* ptr, long l);
void sevseg_print_ul(_7SEGMENT* ptr, unsigned long ul);
void sevseg_print_d(_7SEGMENT* ptr, double d);
void sevseg_print_v(_7SEGMENT* ptr, void);

// Overload sevseg print as per C11 standard
#define sevseg_print(_7SEGMENT* ptr, X) _Generic((X), \
                                                  char: sevseg_print_c, \
                                                  unsigned char: sevseg_print_uc, \
                                                  int: sevseg_print_i, \
                                                  unsigned int: sevseg_print_ui, \
                                                  long: sevseg_print_l, \
                                                  unsigned long: sevseg_print_ul, \
                                                  double: sevseg_print_d, \
                                                  void: sevseg_print_v, \
                                                ) (_7SEGMENT* ptr, X)

void sevseg_println_c(_7SEGMENT* ptr, char c);
void sevseg_println_uc(_7SEGMENT* ptr, unsigned char uc);
void sevseg_println_i(_7SEGMENT* ptr, int i);
void sevseg_println_ui(_7SEGMENT* ptr, unsigned int ui);
void sevseg_println_l(_7SEGMENT* ptr, long l);
void sevseg_println_ul(_7SEGMENT* ptr, unsigned long ul);
void sevseg_println_d(_7SEGMENT* ptr, double d);
void sevseg_println_v(_7SEGMENT* ptr, void);

// Overload sevseg print as per C11 standard
#define sevseg_println(_7SEGMENT* ptr, X) _Generic((X), \
                                                  char: sevseg_println_c, \
                                                  unsigned char: sevseg_println_uc, \
                                                  int: sevseg_println_i, \
                                                  unsigned int: sevseg_println_ui, \
                                                  long: sevseg_println_l, \
                                                  unsigned long: sevseg_println_ul, \
                                                  double: sevseg_println_d, \
                                                  void: sevseg_println_v, \
                                                ) (_7SEGMENT* ptr, X)
*/

void sevseg_writeDigitRaw(_7SEGMENT* ptr, uint8_t x, uint8_t bitmask);
void sevseg_writeDigitNum(_7SEGMENT* ptr, uint8_t x, uint8_t num, uint8_t dot);
void sevseg_drawColon(_7SEGMENT* ptr, uint8_t state);
void sevseg_printNumber(_7SEGMENT* ptr, long n, uint8_t base);
void sevseg_printFloat(_7SEGMENT* ptr, double n, uint8_t fracDigits, uint8_t base);
void sevseg_printError(_7SEGMENT* ptr);

void sevseg_writeColon(_7SEGMENT* ptr);


#endif
