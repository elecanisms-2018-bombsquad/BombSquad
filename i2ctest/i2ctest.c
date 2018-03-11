#include "i2c_reg.h"
#include "ajuart.h"
#include <stdio.h>

int16_t main(void) {
    init_elecanisms();
    i2c_init(157);

    char msg[] = "Hello World!";
    uint8_t len = 13;
    LED3 = 1;


    while (1) {
      // printf("I tried the while loop \n");
      LED1 = !LED1;

      I2Cwritearray(0x7E, msg, len);
      blocking_delay_us(10000);
    }
}
