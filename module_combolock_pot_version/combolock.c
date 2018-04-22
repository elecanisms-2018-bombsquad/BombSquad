#include <p24FJ128GB206.h>
#include "elecanisms.h"
#include "peripheral_core.h"

#define SLAVE_ADDR 0x20

uint16_t new_reading = 0;
uint16_t prev_reading = 0;
uint16_t eval_reading = 0;

uint16_t datasend = 0x25;

void ledoff(){LED1 = 0; LED2 = 0; LED3 = 0; }


void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    prev_reading = new_reading;
    new_reading = read_analog(A0_AN >> 4);
    if (new_reading == prev_reading) {
        eval_reading = new_reading;
    }
}

/////////////////////////////////////////////////

int16_t main(void){
    init_elecanisms();
    i2c_init(157);      // Initializes I2C on I2C3
    I2C3ADD = SLAVE_ADDR>>1;   //initialize the address register
    I2C3MSK = 0;
    _SI2C3IE = 1;       // these two are the same! The underscore is cool

    // Timer 1 Setup
    T1CON = 0x0030;         // set Timer1 period to 0.25s, prescaler 256 match 15624
    PR1 = 0x3D08;
    TMR1 = 0;               // set Timer1 count to 0
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    IEC0bits.T1IE = 1;      // enable Timer1 interrupt
    T1CONbits.TON = 1;      // turn on Timer1


    while(1) {
        complete_flag = 0;
        num_strikes = 0;
        error_code = eval_reading;
        // ledoff();
        // delay_by_nop(30000);
        // datasend = eval_reading;
        // new_reading = read_analog(A0_AN >> 4);

    } // end of while

} //end of main





/*
include <p24FJ128GB206.h>
#include "elecanisms.h"

// #define PIN_ENCODER_A D0
// #define PIN_ENCODER_B D1  /* Remap these to some digital pins from elecanisms.h*/
// //
// static uint8_t enc_prev_pos = 0b0000;
// static uint8_t enc_flags    = 0b0000;
// static uint8_t screen_pos = 0b0000; //what position the active character is on.
// static uint8_t UP = 0b001;
// static uint8_t DOWN = 0b0000;
/*
void delay_by_nop(uint32_t num_nops){    // 1 nop= 375ns
    uint32_t count = 0;
    while (count < num_nops){
        __asm__("nop");
        count +=1;
    }
}
*/ 
/*
void ledoff(){LED1 = 0; LED2 = 0; LED3 = 0; }


int direction = 0;
int last_direction;

int16_t main(void){
    init_elecanisms();
    D8_PUE = IN;
    D9_PUE = IN;
    D8_DIR = 1;
    D9_DIR = 1;

    while(1) {
        // delay_by_nop(30000);
        ledoff();

        if(D9 == 0){
            if(D8 == 0){direction = 5; }
            if(D8 == 1){direction = 10; }
        }

        if(direction == 5){LED1 = 1; }
        if(direction == 10){LED3 = 1;  }

        direction = 0;
    }

}
*/
