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



#include "adafruit_led.h"

static const uint8_t numbertable[] = {
	0x3F, /* 0 */
	0x06, /* 1 */
	0x5B, /* 2 */
	0x4F, /* 3 */
	0x66, /* 4 */
	0x6D, /* 5 */
	0x7D, /* 6 */
	0x07, /* 7 */
	0x7F, /* 8 */
	0x6F, /* 9 */
	0x77, /* a */
	0x7C, /* b */
	0x39, /* C */
	0x5E, /* d */
	0x79, /* E */
	0x71, /* F */
};

static const uint16_t alphafonttable[] PROGMEM =  {

0b0000000000000001,
0b0000000000000010,
0b0000000000000100,
0b0000000000001000,
0b0000000000010000,
0b0000000000100000,
0b0000000001000000,
0b0000000010000000,
0b0000000100000000,
0b0000001000000000,
0b0000010000000000,
0b0000100000000000,
0b0001000000000000,
0b0010000000000000,
0b0100000000000000,
0b1000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0001001011001001,
0b0001010111000000,
0b0001001011111001,
0b0000000011100011,
0b0000010100110000,
0b0001001011001000,
0b0011101000000000,
0b0001011100000000,
0b0000000000000000, //
0b0000000000000110, // !
0b0000001000100000, // "
0b0001001011001110, // #
0b0001001011101101, // $
0b0000110000100100, // %
0b0010001101011101, // &
0b0000010000000000, // '
0b0010010000000000, // (
0b0000100100000000, // )
0b0011111111000000, // *
0b0001001011000000, // +
0b0000100000000000, // ,
0b0000000011000000, // -
0b0000000000000000, // .
0b0000110000000000, // /
0b0000110000111111, // 0
0b0000000000000110, // 1
0b0000000011011011, // 2
0b0000000010001111, // 3
0b0000000011100110, // 4
0b0010000001101001, // 5
0b0000000011111101, // 6
0b0000000000000111, // 7
0b0000000011111111, // 8
0b0000000011101111, // 9
0b0001001000000000, // :
0b0000101000000000, // ;
0b0010010000000000, // <
0b0000000011001000, // =
0b0000100100000000, // >
0b0001000010000011, // ?
0b0000001010111011, // @
0b0000000011110111, // A
0b0001001010001111, // B
0b0000000000111001, // C
0b0001001000001111, // D
0b0000000011111001, // E
0b0000000001110001, // F
0b0000000010111101, // G
0b0000000011110110, // H
0b0001001000000000, // I
0b0000000000011110, // J
0b0010010001110000, // K
0b0000000000111000, // L
0b0000010100110110, // M
0b0010000100110110, // N
0b0000000000111111, // O
0b0000000011110011, // P
0b0010000000111111, // Q
0b0010000011110011, // R
0b0000000011101101, // S
0b0001001000000001, // T
0b0000000000111110, // U
0b0000110000110000, // V
0b0010100000110110, // W
0b0010110100000000, // X
0b0001010100000000, // Y
0b0000110000001001, // Z
0b0000000000111001, // [
0b0010000100000000, //
0b0000000000001111, // ]
0b0000110000000011, // ^
0b0000000000001000, // _
0b0000000100000000, // `
0b0001000001011000, // a
0b0010000001111000, // b
0b0000000011011000, // c
0b0000100010001110, // d
0b0000100001011000, // e
0b0000000001110001, // f
0b0000010010001110, // g
0b0001000001110000, // h
0b0001000000000000, // i
0b0000000000001110, // j
0b0011011000000000, // k
0b0000000000110000, // l
0b0001000011010100, // m
0b0001000001010000, // n
0b0000000011011100, // o
0b0000000101110000, // p
0b0000010010000110, // q
0b0000000001010000, // r
0b0010000010001000, // s
0b0000000001111000, // t
0b0000000000011100, // u
0b0010000000000100, // v
0b0010100000010100, // w
0b0010100011000000, // x
0b0010000000001100, // y
0b0000100001001000, // z
0b0000100101001001, // {
0b0001001000000000, // |
0b0010010010001001, // }
0b0000010100100000, // ~
0b0011111111111111,

};

void led_begin(_ADAFRUIT_LED* ptr, uint8_t addr){
  ptr->i2c_addr = addr;

  i2c_start();
  send_i2c_byte(ptr->i2c_addr);
  send_i2c_byte(0x21); // turn on oscillator
  reset_i2c_bus();
  led_blinkRate((_ADAFRUIT_LED *)ptr, HT16K33_BLINK_OFF);
  led_setBrightness((_ADAFRUIT_LED *)ptr,15); // set max brightness
}

void led_setBrightness(_ADAFRUIT_LED* ptr, uint8_t b){
  if (b > 15) b = 15;
  i2c_start();
  send_i2c_byte(ptr->i2c_addr);
  send_i2c_byte(HT16K33_CMD_BRIGHTNESS | b);
  reset_i2c_bus();
}

void led_blinkRate(_ADAFRUIT_LED* ptr, uint8_t b){
  i2c_start();
  send_i2c_byte(ptr->i2c_addr);
  if (b > 3) b = 0; // turn off if not sure
  send_i2c_byte(HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (b << 1));
  reset_i2c_bus();
}

void led_writeDisplay(_ADAFRUIT_LED* ptr){
  i2c_start();
  send_i2c_byte(ptr->i2c_addr);
	uint8_t i;
  for (i=0; i<8; i++) {
    send_i2c_byte(ptr->displaybuffer[i] & 0xFF);
    send_i2c_byte(ptr->displaybuffer[i] >> 8);
  }
  reset_i2c_bus();
}

void led_clear(_ADAFRUIT_LED* ptr){
	uint8_t i;
	for (i=0; i<8; i++) {
    ptr->displaybuffer[i] = 0;
  }
}

// ALPHANUM ********************************************************************

void alphanum_begin(_ALPHANUM* ptr, uint8_t addr){
  led_begin((_ADAFRUIT_LED *)ptr, addr); // Call super constructor
}

void alphanum_writeDigitRaw(_ALPHANUM* ptr, uint8_t n, uint16_t bitmask){
  ptr->super.displaybuffer[n] = bitmask;
}

void alphanum_writeDigitAscii(_ALPHANUM* ptr, uint8_t n, uint8_t ascii, uint8_t dot){
  uint16_t font = pgm_read_word(alphafonttable+ascii);
  ptr->super.displaybuffer[n] = font;
  if (dot) ptr->super.displaybuffer[n] |= (1<<14);
}

// 24 BARGRAPH *****************************************************************

void bargraph_begin(_BARGRAPH* ptr, uint8_t addr){
  led_begin((_ADAFRUIT_LED *)ptr, addr);
}

void bargraph_setBar(_BARGRAPH* ptr, uint8_t bar, uint8_t color){
  uint16_t a, c;

  if (bar < 12)
    c = bar / 4;
  else
    c = (bar - 12) / 4;

  a = bar % 4;
  if (bar >= 12)
    a += 4;

  //Serial.print("Ano = "); Serial.print(a); Serial.print(" Cath = "); Serial.println(c);
  if (color == LED_RED) {
    // Turn on red LED.
    ptr->super.displaybuffer[c] |= _BV(a);
    // Turn off green LED.
    ptr->super.displaybuffer[c] &= ~_BV(a+8);
  } else if (color == LED_YELLOW) {
    // Turn on red and green LED.
    ptr->super.displaybuffer[c] |= _BV(a) | _BV(a+8);
  } else if (color == LED_OFF) {
    // Turn off red and green LED.
    ptr->super.displaybuffer[c] &= ~_BV(a) & ~_BV(a+8);
  } else if (color == LED_GREEN) {
    // Turn on green LED.
    ptr->super.displaybuffer[c] |= _BV(a+8);
    // Turn off red LED.
    ptr->super.displaybuffer[c] &= ~_BV(a);
  }
}

// 7SEGMENT ********************************************************************

uint8_t write(_7SEGMENT* ptr, uint8_t c){

  uint8_t r = 0;

  if (c == '\n') ptr->position = 0;
  if (c == '\r') ptr->position = 0;

  if ((c >= '0') && (c <= '9')) {
    sevenseg_writeDigitNum(ptr->position, c-'0');
    r = 1;
  }

  ptr->position++;
  if (ptr->position == 2) ptr->position++;

  return r;
}

void sevseg_writeDigitRaw(_7SEGMENT* ptr, uint8_t x, uint8_t bitmask){
  if (x > 4) return;
  ptr->super.displaybuffer[x] = bitmask;
}

void sevseg_writeDigitNum(_7SEGMENT* ptr, uint8_t x, uint8_t num, uint8_t dot){
  if (x > 4) return;
  sevseg_writeDigitRaw(ptr, x, numbertable[num] | (dot << 7));
}

void sevseg_drawColon(_7SEGMENT* ptr, uint8_t state){
  if (state)
    ptr->super.displaybuffer[2] = 0x2;
  else
    ptr->super.displaybuffer[2] = 0;
}

void sevseg_printNumber(_7SEGMENT* ptr, long n, uint8_t base){
  sevseg_printFloat(ptr, n, 0, base);
}

void sevseg_printFloat(_7SEGMENT* ptr, double n, uint8_t fracDigits, uint8_t base){
  uint8_t numericDigits = 4;   // available digits on display
  uint8_t isNegative = 0;  // true if the number is negative

  // is the number negative?
  if(n < 0) {
    isNegative = 1;  // need to draw sign later
    --numericDigits;    // the sign will take up one digit
    n *= -1;            // pretend the number is positive
  }

  // calculate the factor required to shift all fractional digits
  // into the integer part of the number
  double toIntFactor = 1.0;
	int i = 0;
  for(i = 0; i < fracDigits; ++i) toIntFactor *= base;

  // create integer containing digits to display by applying
  // shifting factor and rounding adjustment
  uint32_t displayNumber = n * toIntFactor + 0.5;

  // calculate upper bound on displayNumber given
  // available digits on display
  uint32_t tooBig = 1;
  for(i = 0; i < numericDigits; ++i) tooBig *= base;

  // if displayNumber is too large, try fewer fractional digits
  while(displayNumber >= tooBig) {
    --fracDigits;
    toIntFactor /= base;
    displayNumber = n * toIntFactor + 0.5;
  }

  // did toIntFactor shift the decimal off the display?
  if (toIntFactor < 1) {
    sevseg_printError(ptr);
  } else {
    // otherwise, display the number
    int8_t displayPos = 4;

    if (displayNumber)  //if displayNumber is not 0
    {
			uint8_t i;
      for(i = 0; displayNumber || i <= fracDigits; ++i) {
        uint8_t displayDecimal = (fracDigits != 0 && i == fracDigits);
        sevseg_writeDigitNum(ptr, displayPos--, displayNumber % base, displayDecimal);
        if(displayPos == 2) sevseg_writeDigitRaw(ptr, displayPos--, 0x00);
        displayNumber /= base;
      }
    }
    else {
      sevseg_writeDigitNum(ptr, displayPos--, 0, 0);
    }

    // display negative sign if negative
    if(isNegative) sevseg_writeDigitRaw(ptr, displayPos--, 0x40);

    // clear remaining display positions
    while(displayPos >= 0) sevseg_writeDigitRaw(ptr, displayPos--, 0x00);
  }
}

void sevseg_printError(_7SEGMENT* ptr){
	uint8_t i;
  for(i = 0; i < SEVENSEG_DIGITS; ++i) {
    sevseg_writeDigitRaw(ptr, i, (i == 2 ? 0x00 : 0x40));
  }
}


void sevseg_writeColon(_7SEGMENT* ptr){
  /*
  Wire.beginTransmission(i2c_addr);
  Wire.write((uint8_t)0x04);

  Wire.write(displaybuffer[2] & 0xFF);
  Wire.write(displaybuffer[2] >> 8);

  Wire.endTransmission();
  */
  i2c_start();
  send_i2c_byte(ptr->super.i2c_addr);
  send_i2c_byte((uint8_t)0x04); // start at address $02
  send_i2c_byte(ptr->super.displaybuffer[2] & 0xFF);
  send_i2c_byte(ptr->super.displaybuffer[2] >> 8);
  reset_i2c_bus();
}
