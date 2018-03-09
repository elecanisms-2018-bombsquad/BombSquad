#include "i2c_reg.h"

int16_t main(void) {
    init_elecanisms();
    i2c_init(157);

    char msg[] = "Hello World!";
    uint8_t len = 13;

    while (1) {
      I2Cwritearray(0x70, msg, len);
      blocking_delay_us(10000);
    }
}
