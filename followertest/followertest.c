#include <p24FJ128GB206.h>
#include <stdint.h>
// #include "common.h" // common.h needed for _init_clock
#include "elecanisms.h"
#include "i2c_reg.h"
#include "followertest.h"


#define SLAVE_ADDR 0x60
// #define datasend 0x18

uint16_t waitwritedata;
uint16_t sendreaddata;
uint16_t datasend = 0x05;

typedef enum  {
  STATE_WAIT_FOR_ADDR,
  STATE_WAIT_FOR_WRITE_DATA,
  STATE_SEND_READ_DATA,
  STATE_SEND_READ_LAST
} STATE;

volatile STATE e_mystate = STATE_WAIT_FOR_ADDR;

void clearRCV(void){
    uint8_t bs;
    bs = I2C3RCV;
}

void ledoff(void){
    LED1 = 0;
    LED2 = 0;
    LED3 = 0;
}

// void _ISRFAST _SI2C3Interrupt(void) {
void __attribute__((interrupt, auto_psv)) _SI2C3Interrupt(void) {
    // LED1 = 1;
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

    // case STATE_SEND_READ_DATA:              // read transaction state
    //     I2C3TRN = sendreaddata;        //just keep placing reversed characters in buffer as MASTER reads our I2C port
    //     I2C3CONbits.SCLREL = 1;             //release clock line so MASTER can drive it
    //     e_mystate = STATE_SEND_READ_LAST;  //this is the last character, after byte is shifted out, release the clock line again
    //     // e_mystate = STATE_WAIT_FOR_ADDR;
    //     break;

    case STATE_SEND_READ_LAST:  //interrupt for last character finished shifting out
        e_mystate = STATE_WAIT_FOR_ADDR;
        break;

    default:
        e_mystate = STATE_WAIT_FOR_ADDR;

    } // end of switch statements
}   // end of ISR

int16_t main(void) {
    init_elecanisms();
    i2c_init(157);      // Initializes I2C on I2C3

    I2C3ADD = SLAVE_ADDR>>1;   //initialize the address register
    I2C3MSK = 0;

    // _SI2C1IP = 1;        // Maybe eventially fo this for interrupt priority

    _SI2C3IE = 1;       // these two are the same! The underscore is cool.
    // if(IEC5bits.SI2C3IE == 1){LED2 = 1; }
    // uint8_t bs;
    // bs = I2C3RCV;

    while (1) {
        LED3 = !LED3;
        delay_by_nop(30000);
    }

}
