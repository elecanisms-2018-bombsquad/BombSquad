#include <p24FJ128GB206.h>
#include <stdint.h>
// #include "common.h" // common.h needed for _init_clock
#include "elecanisms.h"
#include "i2c_reg.h"
#include "leadertest.h"

int16_t main(void) {
    init_elecanisms();
    i2c_init(1e3);    // Initializes I2C on I2C3

    _ISQUC leader1;

    i2c_start();

    int16_t pollval = I2Cpoll(18);

    if(pollval == 0){
        LED1 = 1;
    }

    delay_by_nop(50);

    if(pollval == 1){
        LED3 = 1;
    }

    delay_by_nop(50);

}
