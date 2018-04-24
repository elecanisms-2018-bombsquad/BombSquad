#include <p24FJ128GB206.h>
#include "elecanisms.h"
#include "peripheral_core.h"
#include "ajuart.h"
#include "adafruit_led.h"

#define SLAVE_ADDR 0x60

uint16_t new_reading = 0;
uint16_t prev_reading = 0;
uint16_t eval_reading = 0;
uint8_t tmp;
uint8_t a = 1;

_7SEGMENT matrix;

const uint8_t target_addr = 0xE0;

void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    prev_reading = new_reading;
    new_reading = read_analog(A0_AN) >> 5 ;
    if (new_reading == prev_reading) {
        eval_reading = new_reading;
    }
}

void dispNumber(uint16_t number) {
    uint8_t num_new;
    uint8_t thousands, hundreds, tens, ones;

    thousands = number / 1000 ;
    num_new = number - ( thousands * 1000);
    hundreds = num_new  / 100 ;
    num_new = num_new - (hundreds * 100);
    tens = num_new / 10 ;
    num_new = num_new - (tens * 10) ;
    ones = num_new ;

    U1_putc(thousands);
    U1_putc(hundreds);
    U1_putc(tens);
    U1_putc(ones);
    U1_putc('\r');
    U1_putc('\n');
    U1_flush_tx_buffer();

    // sevseg_writeDigitNum(&matrix, 0, thousands, 0);
    // sevseg_writeDigitNum(&matrix, 1, hundreds, 0);
    // sevseg_writeDigitNum(&matrix, 3, tens, 0);
    // sevseg_writeDigitNum(&matrix, 4, ones, 0);

    // sevseg_writeDigitNum(&matrix, 0, thousands, 0);
    sevseg_writeDigitNum(&matrix, 1, tens, 0);
    sevseg_writeDigitNum(&matrix, 3, ones, 0);
    // sevseg_writeDigitNum(&matrix, 4, ones, 0);

    led_writeDisplay((_ADAFRUIT_LED*)&matrix.super); //Don't forget to actually write the data!
}

/////////////////////////////////////////////////

int16_t main(void){
    init_elecanisms();
    init_ajuart();
    i2c2_init(157);     // Initializes I2C2 for master communicaiton SCL D2// SDA D3
    i2c_init(1e3);      // initializes I2C3 for screen thing
    led_begin((_ADAFRUIT_LED*)&matrix.super, target_addr); // Set up the HT16K33 and start the oscillator

    I2C2ADD = SLAVE_ADDR>>1;   //initialize the address register
    I2C2MSK = 0;
    _SI2C2IE = 1;       // these two are the same! The underscore is cool

    // Timer 1 Setup
    T1CON = 0x0020;         // set Timer1cd .. period to 10 ms for debounce
    PR1 = 0x2710;           // prescaler 16, match value 10000
    // T1CON = 0x0030; // PR1 = 0x3D08;        // set Timer1 period to 0.25s, prescaler 256 match 15624
    TMR1 = 0;               // set Timer1 count to 0
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    IEC0bits.T1IE = 1;      // enable Timer1 interrupt
    T1CONbits.TON = 1;      // turn on Timer1


    while(1) {
        complete_flag = eval_reading & 0b10000000;
        num_strikes = eval_reading & 0b01110000;
        error_code = eval_reading & 0b00001111;
        if (eval_reading % 3 == 1){
            LED1 = 1;
            // LED3 = 0;
        }
        else { LED1 = 0; }

        dispNumber(eval_reading);

        // tmp[0] = eval_reading >> 8; tmp[1] = eva
        // U1_putc(tmp[0]);
        // U1_putc(tmp[1]);
        // tmp = (uint8_t)eval_reading;
        // U1_putc(eval_reading);
        // U1_flush_tx_buffer();
        // LED3 = !LED3;
        // delay_by_nop(30000);

    } // end of while loop
} //end of main


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

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
