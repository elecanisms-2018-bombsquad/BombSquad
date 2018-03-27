#include <p24FJ128GB206.h>
#include <stdint.h>
// #include "common.h" // common.h needed for _init_clock
#include "elecanisms.h"
#include "i2c_reg.h"
#include "followertest.h"

int16_t main(void) {
    // init_clock();
    init_elecanisms();
    i2c_init(1e3);      // Initializes I2C on I2C3

    _ISQUC follower1;
    
    // set address of follower
    I2C3ADD = 18;
    I2C3MSK = 0;


}
