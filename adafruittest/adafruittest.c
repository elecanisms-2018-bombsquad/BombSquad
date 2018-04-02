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
    sevseg_drawColon(&matrix, drawDots);
    sevseg_writeDigitNum(&matrix, 3, (counter / 10) % 10, drawDots);
    sevseg_writeDigitNum(&matrix, 4, counter % 10, drawDots);

    blinkcounter+=50;
    if (blinkcounter < 500) {
      drawDots = 0;
    } else if (blinkcounter < 1000) {
      drawDots = 1;
    } else {
      blinkcounter = 0;
    }
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

  LED2 = ON;
  while (LED2) {
  delay_by_nop(300000);
  LED2 = I2Cpoll(target_addr);
  }

  led_begin((_ADAFRUIT_LED*)&matrix.super, target_addr);
  // led_blinkRate((_ADAFRUIT_LED*)&matrix.super, HT16K33_BLINK_OFF);

  while (1){
    drawOnce();
    LED3 = !LED3;
    delay_by_nop(3000000);
  }
  // while (1) {
  //   LED3 = !LED3;
  //   delay_by_nop(300000);
  // }
}
