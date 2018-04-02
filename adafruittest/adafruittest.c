#include <p24FJ128GB206.h>
#include <stdint.h>

#include "common.h" // common.h needed for _init_clock
#include "elecanisms.h"
#include "i2c_reg.h"
#include "adafruit_led.h"

_7SEGMENT matrix;

const uint8_t target_addr = 0xE0;


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

void drawOnce(void) {
  sevseg_writeDigitNum(&matrix, 0, 1, 0);
  sevseg_writeDigitNum(&matrix, 1, 2, 0);
  sevseg_writeDigitNum(&matrix, 3, 3, 0);
  sevseg_writeDigitNum(&matrix, 4, 4, 0);
  led_writeDisplay((_ADAFRUIT_LED*)&matrix.super);
}

int16_t main(void) {
  init_elecanisms();

  init_clock();
  // Initializes I2C on I2C3
  i2c_init(1e3);

  LED2 = ON; // Start LED2 on
  while (LED2) {
    delay_by_nop(300000); // wait to make LED2 visible
    LED2 = I2Cpoll(target_addr); // turn off LED2 if we've found the display
  }

  led_begin((_ADAFRUIT_LED*)&matrix.super, target_addr); // Set up the HT16K33 and start the oscillator

  while (1){
    // drawOnce(); // Draw 1234
    countup(); // Count up to 9999
    LED3 = !LED3;
    delay_by_nop(3000000);
  }
}
