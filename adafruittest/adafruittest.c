#include <p24FJ128GB206.h>
#include <stdint.h>

#include "common.h" // common.h needed for _init_clock
#include "elecanisms.h"
#include "i2c_reg.h"
#include "adafruit_led.h"

_7SEGMENT matrix;
_BARGRAPH bar;
_ALPHANUM alpha;

const uint8_t matrix_addr = 0xE0;
const uint8_t bar_addr = 0xE8;
const uint8_t alpha_addr = 0xE0;

void countup(void) {
  // Draw each digit
  uint16_t blinkcounter = 0;
  uint8_t drawDots = 0;
  uint16_t counter;
  for (counter = 0; counter < 9999; counter ++) {
    sevseg_writeDigitNum(&matrix, 0, (counter / 1000), drawDots);
    sevseg_writeDigitNum(&matrix, 1, (counter / 100) % 10, drawDots);
    sevseg_writeDigitNum(&matrix, 3, (counter / 10) % 10, drawDots);
    sevseg_writeDigitNum(&matrix, 4, counter % 10, drawDots);

    led_writeDisplay((_ADAFRUIT_LED*)&matrix.super);
  }
}

// Display a seconds variable in minutes and seconds on the seven segment
void dispSeconds(uint16_t seconds) {
  // Turn seconds into minutes and seconds
  uint16_t minutes = seconds / 60;
  uint8_t displaySeconds = seconds % 60;
  sevseg_writeDigitNum(&matrix, 0, minutes / 10, 0);
  sevseg_writeDigitNum(&matrix, 1, minutes % 10, 0);
  sevseg_drawColon(&matrix, 1); // Times are supposed to have a colon I guess
  sevseg_writeDigitNum(&matrix, 3, (displaySeconds / 10) % 10, 0);
  sevseg_writeDigitNum(&matrix, 4, displaySeconds % 10, 0);

  led_writeDisplay((_ADAFRUIT_LED*)&matrix.super); //Don't forget to actually write the data!
}

// Draw 1234 on the seven segment
void drawAlphanum(void) {
  sevseg_writeDigitNum(&matrix, 0, 1, 0); //pointer, index, character to write, dot?
  sevseg_writeDigitNum(&matrix, 1, 2, 0);
  sevseg_writeDigitNum(&matrix, 3, 3, 0);
  sevseg_writeDigitNum(&matrix, 4, 4, 0);
  led_writeDisplay((_ADAFRUIT_LED*)&matrix.super);
}

// Draw the whole bar 1 color
void drawAll(uint8_t color) {
    uint8_t i;
    for(i = 0; i < 24; i++) {
        bargraph_setBar(&bar, i, color);
    }
    led_writeDisplay((_ADAFRUIT_LED*)&bar.super);
}

// Draw ABCD on the alphanumeric displaybuffer
void drawABCD(void) {
    alphanum_writeDigitAscii(&alpha, 0, 'A', 0); //pointer, index, character to write, dot
    alphanum_writeDigitAscii(&alpha, 1, 'B', 0);
    alphanum_writeDigitAscii(&alpha, 2, 'C', 0);
    alphanum_writeDigitAscii(&alpha, 3, 'D', 0);
    led_writeDisplay((_ADAFRUIT_LED*)&alpha.super);
}

// iterate through characters on the alphanumeric displaybuffer
void iterateAlpha(void) {
    uint8_t base_char = 0;
    for(base_char = 0; base_char < 125; base_char++) {
        alphanum_writeDigitAscii(&alpha, 0, base_char, 0); //pointer, index, character to write, dot
        alphanum_writeDigitAscii(&alpha, 1, (base_char+1), 0);
        alphanum_writeDigitAscii(&alpha, 2, (base_char+2), 0);
        alphanum_writeDigitAscii(&alpha, 3, (base_char+3), 0);
        led_writeDisplay((_ADAFRUIT_LED*)&alpha.super);
        delay_by_nop(100000);
    }
}

// Draw a single character;
void drawChar(uint8_t ch) {
    alphanum_writeDigitAscii(&alpha, 0, ch, 0);
    led_writeDisplay((_ADAFRUIT_LED*)&alpha.super);
}

int16_t main(void) {
  init_elecanisms();

  init_clock();
  // Initializes I2C on I2C3
  i2c_init(1e3);


  LED2 = ON; // Start LED2 on
  while (LED2) {
    delay_by_nop(300000); // wait to make LED2 visible

    /* Select device to test here'  */
    // if( I2Cpoll(matrix_addr)) LED2 = OFF; // turn off LED2 if we've found the display
    // if( I2Cpoll(bar_addr)) LED2 = OFF; // turn off LED2 if we've found the display
    if( I2Cpoll(alpha_addr)) LED2 = OFF; // turn off LED2 if we've found the display

  }

  /* And here */
  // led_begin((_ADAFRUIT_LED*)&matrix.super, matrix_addr); // Set up the HT16K33 and start the oscillator
  // led_begin((_ADAFRUIT_LED*)&bar.super, bar_addr); // Set up the HT16K33 and start the oscillator
  led_begin((_ADAFRUIT_LED*)&alpha.super, alpha_addr); // Set up the HT16K33 and start the oscillator


  uint16_t timeleft = 240;
  uint8_t col = 0;
  while (1){

    /* And also here */
    // dispSeconds(timeleft);
    // countup();

    // col = (col+1) % 4;
    // drawAll(col); // iterate through colors

    // drawABCD();
    iterateAlpha();
    // drawChar('%');

    delay_by_nop(2666666); // Delay approximately a second (1s / 375ns) = 2666666
    timeleft--;
    if (timeleft == 0) {
      timeleft = 240;
    }
  }
}
