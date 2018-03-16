/*
** Test for responding to i2c addressing
*/
#include "elecanisms.h"

int16_t main(void) {
    init_elecanisms();
    i2c_init(1e3);
    I2C3CONbits.I2CEN = 0;

    I2C3MSK = 0x0;
    I2C3ADD = 0x22;
    I2C3CONbits.I2CEN = 0;

    while (1) {
      LED1 = !LED1;
      delay_by_nop(500000);
    }
}
