#include "elecanisms.h"
#include "peripheral_core.h"

#define son       1
#define soff      0

#define SLAVE_ADDR 0x60

uint16_t state;

//////////////////////

void ledoff(void) {
    LED1 = 0;delay_by_nop(1);
    LED2 = 0; delay_by_nop(1);
    LED3 = 0; delay_by_nop(1);
    D0 = OFF;
}

int16_t main(void) {

    init_elecanisms();
    init_ajuart();
    i2c2_init(157);      // Initializes I2C on I2C3
    I2C2ADD = SLAVE_ADDR>>1;   //initialize the address register
    I2C2MSK = 0;
    _SI2C2IE = 1;       // these two are the same! The underscore is cool


    while (1) {
        delay_by_nop(30000);
        uint16_t val = read_analog(A0_AN);

        if( val > 500 ) {state = son;}    //n
        if( val < 500 ) {state = soff;}                 //o



        switch(state){

            case son:
                complete_flag = 0x01;
                ledoff();
                LED1 = ON;
                break;
            case soff:
                complete_flag = 0x00;
                ledoff();
                LED2 = ON;
                break;



        } // end of switch statement
    } // end of while
} // end of main
