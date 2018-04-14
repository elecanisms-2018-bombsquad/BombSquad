#include "peripheral_core.h"
#include "i2c_reg.h"


// #define SLAVE_ADDR 0x60

uint16_t waitwritedata;
uint16_t sendreaddata;
// uint16_t datasend = 0x05;
uint16_t datasend;


volatile STATE e_mystate = STATE_WAIT_FOR_ADDR;


void __attribute__((interrupt, auto_psv)) _SI2C3Interrupt(void) {
    uint8_t u8_c;
    _SI2C3IF = 0;

    switch (e_mystate) {

    case STATE_WAIT_FOR_ADDR:
        u8_c = I2C3RCV;     //clear RBF bit for address
        if (I2C3STATbits.R_W) {     // if data transfer is out from slave // check R/W bit of address byte for read or write transaction
            I2C3TRN = datasend;   //Set transmit register to first data byte to send back to master
            I2C3CONbits.SCLREL = 1;     //release clock line so MASTER can drive it// SCL released during slave read
            e_mystate = STATE_SEND_READ_LAST; //goes to read transaction
        }
        else e_mystate = STATE_WAIT_FOR_WRITE_DATA;     // goes to write transaction
        break;

    case STATE_WAIT_FOR_WRITE_DATA:     // write transaction state    //character arrived, place in buffer
        waitwritedata = I2C3RCV;        //read the byte
        e_mystate = STATE_WAIT_FOR_ADDR;    // go to wait for next transaction
        break;

    case STATE_SEND_READ_LAST:  //interrupt for last character finished shifting out
        e_mystate = STATE_WAIT_FOR_ADDR;
        break;

    default:
        e_mystate = STATE_WAIT_FOR_ADDR;

    } // end of switch statements
}   // end of ISR
