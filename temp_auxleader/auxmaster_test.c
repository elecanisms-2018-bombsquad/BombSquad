//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#include <p24FJ128GB206.h>
#include <stdint.h>
#include "elecanisms.h"
#include "i2c_reg.h"

#define SLAVE_ADDR 0x60

uint16_t datareturned;

int main(void) {
    init_elecanisms();
    uint16_t u16_len;
    uint16_t pollval;
    int16_t i = 0;
    i2c_init(157);      // initialize I2C for 16Mhz OSC with 100kHz I2C clock

    delay_by_nop(3000);

    while(1) {
        datareturned = 0;
        delay_by_nop(500);

        i2c_start();
        send_i2c_byte(SLAVE_ADDR | 1);  // init a read, last to 1
        datareturned = i2c_read_nack();
        // if(datareturned > 1) {LED1 = 1;}
        if(datareturned == 0b10000000) {LED2 = 1;}
        if(datareturned == 0b00000000) {LED3 = 1;}
        reset_i2c_bus();

        delay_by_nop(1000000);
        LED1 = 0; LED2 = 0; LED3 = 0;
        delay_by_nop(1000000);


        LED1 = 1;
        i2c_start();
        send_i2c_byte(SLAVE_ADDR | 0);
        delay_by_nop(50);
        send_i2c_byte(0b10000000);
        reset_i2c_bus();
        delay_by_nop(1000000);

        LED1 = 0; LED2 = 0; LED3 = 0;




        // datareturned = I2Cread(SLAVE_ADDR, 1);
        // writeNI2C1(SLAVE_I2C_ADDR,(uint8_t *)sz_1,u16_len);   //send the string
        // readNI2C1(SLAVE_I2C_ADDR, (uint8_t *) sz_1,u16_len) ;  //read the reversed string

    }
}





/*
#include <p24FJ128GB206.h>
#include <stdint.h>
#include "elecanisms.h"
#include "i2c_reg.h"
// #include "leadertest.h"

#define SLAVE_ADDR 0x08

uint16_t datareturned;

void clearRCV(void){
    uint8_t bs;
    bs = I2C3RCV;
}

int main(void) {
    init_elecanisms();
    uint16_t u16_len;
    uint16_t pollval;
    int16_t i = 0;
    i2c_init(157);      // initialize I2C for 16Mhz OSC with 100kHz I2C clock

    delay_by_nop(3000);

    while(1) {
        datareturned = 0;
        delay_by_nop(500);

        i2c_start();
        send_i2c_byte(SLAVE_ADDR | 1);  // init a read, last to 1
        datareturned = i2c_read_nack();
        if(datareturned == 3){LED1 = 1;}
        if(datareturned == 5){LED3 = 1;}
        if(datareturned == 7){LED2 = 1;}
        reset_i2c_bus();

        delay_by_nop(90000);

    }
}

*/
