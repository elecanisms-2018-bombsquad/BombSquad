#include "i2c_reg.h"
#include "ajuart.h"
#include <stdio.h>

void delay_by_nop(int num_nops){    // 1 nop= 375ns
    uint16_t count = 0;
    while (count < num_nops){
        __asm__("nop");
        count +=1;
    }
}

int16_t main(void) {
    init_elecanisms();
    LED1 = 1;
    delay_by_nop(50);
    i2c_init(157);
    LED3 = 1;

    char msg[] = "imiy}y";
    uint8_t len = 6;

    while (1) {
        reset_i2c_bus();

      I2Cwritearray(0x7C, msg, len);
    }
}
