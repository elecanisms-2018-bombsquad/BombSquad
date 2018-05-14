#include <stdlib.h>
#include <stdio.h>
#include <string.h> // for strlen, memset
#include "elecanisms.h"
#include "adafruit_led.h"
#include "i2c_reg.h"
#include "lcd.h"
#include "ajuart.h"
#include "i2c_address_space.h"
#include "bs_headers.h"
#include "master.h"

#define STRIKE1_RLED      D0
#define STRIKE1_GLED      D1
#define STRIKE2_RLED      D10
#define STRIKE2_GLED      D11
#define STRIKE3_RLED      D4
#define STRIKE3_GLED      D5
#define FLUX_LED          D7 // Blue
#define EPS_LED           D6 // Red
#define RTC_LED           D8 // Green

#define max_time 300

uint16_t time_left;

_7SEGMENT matrix;
uint8_t matrix_addr = 0xE0;

_LCD lcd1;
char* dispstring;

char char_buffer[128];
uint8_t datareturned;
uint8_t peripheral_addrs[7] = {TEST_PERIPHERAL_ADDR,
                               MODULE_CODEWORD_ADDR,
                               MODULE_AUXCABLE_ADDR,
                               MODULE_BUTTON_ADDR  ,
                               MODULE_NEEDY_ADDR   ,
                               MODULE_SIMON_ADDR   ,
                               MODULE_MORSE_ADDR   };

uint8_t peripheral_present[7] = {0,0,0,0,0,0,0};
uint8_t peripheral_complete[7] = {0,0,0,0,0,0,0};
uint8_t num_strikes = 0;
uint8_t prev_num_strikes = 0;
uint8_t game_complete = 0;
uint8_t serial_idx = 0;
uint16_t rand_val;

const uint16_t PWM_PERIOD_2_3 = (uint16_t)(FCY / 2.3e3 - 1.);
const uint16_t PWM_PERIOD_1_9 = (uint16_t)(FCY / 1.9e3 - 1.);
const uint16_t PWM_PERIOD_1 = (uint16_t)  (FCY / 1e3 - 1.);
const uint16_t PWM_PERIOD_2 = (uint16_t)  (FCY / 2e3 - 1.);
const uint16_t PWM_PERIOD_40 = (uint16_t) (FCY / 4 - 1.);


// Forward declarations of functions to avoid a header file :/
void idle(void);
void run(void);
void end_win(void);
void end_fail(void);
void dispSeconds(uint16_t seconds);

typedef void (*STATE_HANDLER_T)(void);
STATE_HANDLER_T state, last_state;

void strikeLEDOff() {
    STRIKE1_RLED = OFF; delay_by_nop(1);
    STRIKE1_GLED = OFF; delay_by_nop(1);
    STRIKE2_RLED = OFF; delay_by_nop(1);
    STRIKE2_GLED = OFF; delay_by_nop(1);
    STRIKE3_RLED = OFF; delay_by_nop(1);
    STRIKE3_GLED = OFF; delay_by_nop(1);
}

// MAIN FUNCTION ***************************************************************

int16_t main(void) {
    init_elecanisms();
    init_clock(); // not sure if this does anything
    init_ajuart();
    // Initializes I2C on I2C3
    i2c_init(1e3);
    led_begin((_ADAFRUIT_LED*)&matrix.super, matrix_addr); // Set up the HT16K33 and start the oscillator
    lcd_init(&lcd1, 0x05, 'A'); //Setup LCD screen (type A i/o extender)
    lcd_clear(&lcd1);  // Clears _LCD objects from previous array

    time_left = max_time; // Set up time left to be max time

    // Setup pins as output
    D0_DIR = 0;
    D1_DIR = 0;
    D10_DIR = 0;
    D11_DIR = 0;
    D4_DIR = 0;
    D5_DIR = 0;
    D6_DIR = 0;
    D7_DIR = 0;
    D8_DIR = 0;
    D9_DIR = 0;

    // // Setup D8 as input
    // D8_DIR = 1;
    // // Set up pull-up resistor on button D8
    // CNPU4bits.CN54PUE = 1;
    // // Set D9 as pull-down for button
    // D9 = 0;

    /* Timer 1 setup for game timing*/
    T1CON = 0x0030;         // set Timer1 period to 1s
    PR1 = 0xF422;
    TMR1 = 0;               // set Timer1 count to 0
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    IEC0bits.T1IE = 1;      // enable Timer1 interrupt

    /* Timer 2 setup for beep timing*/
    T2CON = 0x0030;
    PR2 = 0x186A;           // set Timer2 period to 0.1s
    TMR2 = 0;               // set Timer1 count to 0
    IFS0bits.T2IF = 0;      // lower T2 interrupt flag
    IEC0bits.T2IE = 1;      // enable T2 interrupt

    /* Remap OC1 to D11 */
    uint8_t *RPOR, *RPINR;
    RPOR = (uint8_t *)&RPOR0;
    RPINR = (uint8_t *)&RPINR0;

    __builtin_write_OSCCONL(OSCCON & 0xBF);
    RPOR[D12_RP] = OC1_RP;  // connect the OC1 module output to pin D8
    __builtin_write_OSCCONL(OSCCON | 0x40);

    /* OC1 setup for beep output*/
    OC1CON1bits.OCTSEL = 0b111;   // configure OC1 module to use the peripheral
                                  //   clock (i.e., FCY, OCTSEL<2:0> = 0b111) and
    OC1CON1bits.OCM = 0b110;      //   and to operate in edge-aligned PWM mode
                                  //   (OCM<2:0> = 0b110)
    OC1CON2bits.OCTRIG = 0;       // configure OC1 module to syncrhonize to itself
    OC1CON2bits.SYNCSEL = 0x1F;   //   (i.e., OCTRIG = 0 and SYNCSEL<4:0> = 0b11111)

    OC1RS = PWM_PERIOD_2_3;       // configure period register to get 2.3 kHz
    OC1TMR = 0;                   // set OC1 timer count to 0
    OC1R = 0;                     // start with it off


    state = idle;           // Initialize state to idle
    last_state = (STATE_HANDLER_T)NULL;

    num_strikes = 0;
    prev_num_strikes = 0;
    game_complete = 0;

    /* Setup LEDs and serial number */
    rand_val = read_analog(A0_AN); // Set up the seed

    // Add more random noise
    uint8_t i, j;
    for (i=0; i<20; i++) {
        for (j=0; j<read_analog(A0_AN); j++) {
            rand_next();
            delay_by_nop(read_analog(A0_AN));
        }
    }

    serial_idx = rand_val%6;

    char _dispstring[17] = "";
    for (i=0; i<16; i++){
        _dispstring[i] = serial_nums[serial_idx][i];
    }
    dispstring = _dispstring;
    lcd_print2(&lcd1, dispstring, "");

    rand_next();
    sprintf(char_buffer, "LED rand val:%d, %d, %d", ((rand_val%8) & 0x1), ((rand_val%8) & 0x2), ((rand_val%8) & 0x4));
    U1_puts(char_buffer);
    U1_putc('\r');
    U1_putc('\n');
    U1_flush_tx_buffer();


    RTC_LED  = ((rand_val%8) & 0x1) ? ON : OFF; delay_by_nop(1);
    FLUX_LED = ((rand_val%8) & 0x2) ? ON : OFF; delay_by_nop(1);
    EPS_LED  = ((rand_val%8) & 0x4) ? ON : OFF; delay_by_nop(1);


    i2c2_init(157);      // initialize I2C for 16Mhz OSC with 100kHz I2C clock

    delay_by_nop(300000);

    // Poll the peripherals to see who's here
    for (i = 0; i < 7; i++) {
        uint8_t temp = 0;

        i2c2_start();
        temp = send_i2c2_byte(peripheral_addrs[i]); // set /W bit
        if (temp == 0) {
            peripheral_present[i] = 1; // We found it, but we need to reset the FSM for i2c
            send_i2c2_byte(0xA0); // Send dummy byte to reset FSM
        }
        reset_i2c2_bus();
    }

    /* Send out parameters */
    for (i = 0; i < 7; i++) {
        if (peripheral_present[i]) {
            i2c2_start();
            send_i2c2_byte(peripheral_addrs[i] | 0);  // init a write, last to 0
            send_i2c2_byte((HEADER_SERIAL_NUMBER << 5) | serial_idx); // Serial Number
            reset_i2c2_bus();
        }
    }

    for (i = 0; i < 7; i++) {
        if (peripheral_present[i]) {
            i2c2_start();
            send_i2c2_byte(peripheral_addrs[i] | 0);  // init a write, last to 0
            send_i2c2_byte((HEADER_MASTER_LED << 5) | rand_val%8); // LEDs (should be fine, might break if rand_val changes)
            reset_i2c2_bus();
        }
    }

    /* Send out start condition */
    for (i = 0; i < 7; i++) {
        if (peripheral_present[i]) {
            i2c2_start();
            send_i2c2_byte(peripheral_addrs[i] | 0);  // init a write, last to 0
            send_i2c2_byte(HEADER_START_GAME << 5);
            reset_i2c2_bus();
        }
    }

    while (1) {
        state();
    }

}

// ISRs ************************************************************************

uint8_t beep_state = 0;

void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0;  // if it's been a second, lower the counter and show it
    time_left--;
    if (state == run) {
        TMR2 = 0;
        beep_state = 0;
        IEC0bits.T2IE = 1; // enable t2 interrupt
        T2CONbits.TON = 1; //Start T2
        OC1RS = PWM_PERIOD_2_3;
        OC1R = OC1RS>>1; // start beep
    }
}

void __attribute__((interrupt, auto_psv)) _T2Interrupt(void) {
    IFS0bits.T2IF = 0;
    switch (beep_state) {
        case 0:         // turn off after first beep
            OC1R = 0;
            beep_state++;
            LED3 = OFF; delay_by_nop(1);
            LED1 = ON; delay_by_nop(1);
            break;
        case 1:
            OC1RS = PWM_PERIOD_1_9; // make lower beep
            OC1R = OC1RS>>1;
            LED1 = OFF; delay_by_nop(1);
            LED2 = ON; delay_by_nop(1);
            beep_state++;
            break;
        case 2:
            OC1RS = PWM_PERIOD_2_3; // Reset to high beep and stop
            OC1R = 0;
            LED1 = OFF; delay_by_nop(1);
            LED2 = ON; delay_by_nop(1);
            beep_state++;
            break;
        default:
            break;
    }
}

// STATE MACHINE FUNCTIONS *****************************************************

void idle(void) {
    if (state != last_state) {  // if we are entering the state, do initialization stuff
        last_state = state;
        dispSeconds(max_time); // Start by showing full time
    }

    // Perform state tasks

    // Check for state transitions
    /* TODO is it better to not have the start button? */
    delay_by_nop(2000000); // delay for a long time at the start
    state=run;

    U1_puts("Setup");
    U1_putc('\r');
    U1_putc('\n');
    U1_flush_tx_buffer();

    // if (D8 == 0) { // D8 is pulled-up, if button is pressed it pulls it down
    //     state = run;
    // }

    // if we are leaving the state, do clean up stuff
    if (state != last_state) {
    }
}

void run(void) {
    if (state != last_state) {  // if we are entering the state, do intitialization stuff
        last_state = state;
        IFS0bits.T1IF = 0; //lower interrupt flag
        TMR1 = 0;          // reset timer register
        T1CONbits.TON = 1; // enable 1 second timer

        time_left = max_time;
        dispSeconds(time_left);
        STRIKE1_GLED = ON; // Turn on strike LEDs to green
        delay_by_nop(1);
        STRIKE2_GLED = ON;
        delay_by_nop(1);
        STRIKE3_GLED = ON;
        U1_puts("run");
        U1_putc('\r');
        U1_putc('\n');
        U1_flush_tx_buffer();
    }

    // Perform state tasks
    datareturned = 0;
    uint8_t i;
    uint16_t prev_r = 0; uint16_t prev_rs = 0;

    // Handle time
    dispSeconds(time_left);
    if (time_left == 0) {
        for (i = 0; i < 7; i++) {
            if(peripheral_present[i]) {
                i2c2_start();
                send_i2c2_byte(peripheral_addrs[i] | 0);  // init a write, last to 0
                send_i2c2_byte(HEADER_END_LOSE << 5); // Broadcast the current number of strikes
                reset_i2c2_bus();
            }
        }
       state = end_fail;            // if we're out of time, go to end
    }

    // Get completeness and strikes from every module
    prev_num_strikes = num_strikes;
    for (i = 0; i < 7; i++) {
        if(peripheral_present[i]) {
            i2c2_start();
            send_i2c2_byte(peripheral_addrs[i] | 1);  // init a read, last to 1
            datareturned = i2c2_read_nack();
            reset_i2c2_bus();

            if (datareturned & 0b10000000) { // Complete flag
                if (peripheral_complete[i] == 0){

                    disable_interrupts();
                    prev_rs = OC1RS;
                    prev_r = OC1R;
                    OC1RS = PWM_PERIOD_2;
                    OC1R = OC1RS >> 1; // Make a high-pitched complete sound
                    delay_by_nop(300000);
                    OC1RS = prev_rs;
                    OC1R = prev_r;
                    enable_interrupts();
                }
                peripheral_complete[i] = 1;
            }
            if (((datareturned & 0b01110000) >> 4) > prev_num_strikes) { //If the module recorded any strikes
                num_strikes = ((datareturned & 0b01110000) >> 4);
            }
            if ((datareturned & 0b00001111) != 0) {
                // TODO: implement error codes if necessary
            }
        }
    }
    //Handles completeness
    game_complete = 1;
    for (i = 0; i < 7; i++) {
        if (peripheral_present[i] && !peripheral_complete[i]) {
            game_complete = 0;
        }
    }
    // If we checked all of them and the game is still complete, then count it!
    if (game_complete) {
        for (i = 0; i < 7; i++) {
            if(peripheral_present[i]) {
                i2c2_start();
                send_i2c2_byte(peripheral_addrs[i] | 0);  // init a write, last to 0
                send_i2c2_byte(HEADER_END_WIN << 5); // Broadcast that we won
                reset_i2c2_bus();
            }
        }
        //TODO: Go to 'win' state
        state = end_win;
    }

    //Handles strikes
    strikeLEDOff(); // Handle strike LEDs
    switch (num_strikes){
        case 0:
            STRIKE1_GLED = ON; delay_by_nop(1);
            STRIKE2_GLED = ON; delay_by_nop(1);
            STRIKE3_GLED = ON; delay_by_nop(1);
        break;
        case 1:
            STRIKE1_RLED = ON; delay_by_nop(1);
            STRIKE2_GLED = ON; delay_by_nop(1);
            STRIKE3_GLED = ON; delay_by_nop(1);
        break;
        case 2:
            STRIKE1_RLED = ON; delay_by_nop(1);
            STRIKE2_RLED = ON; delay_by_nop(1);
            STRIKE3_GLED = ON; delay_by_nop(1);
        break;
        default:
            STRIKE1_RLED = ON; delay_by_nop(1);
            STRIKE2_RLED = ON; delay_by_nop(1);
            STRIKE3_RLED = ON; delay_by_nop(1);
        break;
    }

    if (num_strikes > prev_num_strikes) {
        for (i = 0; i < 7; i++) {
            if (peripheral_present[i]) {
                i2c2_start();
                send_i2c2_byte(peripheral_addrs[i] | 0);  // init a write, last to 0
                send_i2c2_byte((HEADER_NUM_STRIKES<<5) | num_strikes); // Broadcast the current number of strikes
                reset_i2c2_bus();
            }
        }
        sprintf(char_buffer, "Num Strikes:%d", num_strikes);
        U1_puts(char_buffer);
        U1_putc('\r');
        U1_putc('\n');
        U1_flush_tx_buffer();
        disable_interrupts();
        prev_rs = OC1RS;
        prev_r = OC1R;
        OC1RS = PWM_PERIOD_1;
        OC1R = OC1RS >> 1; // Make a soise for the strike
        delay_by_nop(300000);
        OC1RS = prev_rs;
        OC1R = prev_r;
        enable_interrupts();
    }
    if (num_strikes > 2) {
        for (i = 0; i < 7; i++) {
            if(peripheral_present[i]) {
                i2c2_start();
                send_i2c2_byte(peripheral_addrs[i] | 0);  // init a write, last to 0
                send_i2c2_byte(HEADER_END_LOSE << 5); // Broadcast the current number of strikes
                reset_i2c2_bus();
            }
        }
        state = end_fail;
    }

    // Check for state transitions
    if (SW2 == 0) {
        state = idle;           // SW2 resets to idle
    }

    // if we are leaving the state, do clean up stuff
    if (state != last_state) {
        T1CONbits.TON = 0;
        STRIKE1_GLED = OFF; // Turn off strike LEDs.
        delay_by_nop(1);
        STRIKE2_GLED = OFF;
        delay_by_nop(1);
        STRIKE3_GLED = OFF;
        IEC0bits.T2IE = 0; // disable beep interrupt
        OC1R = 0; // turn off beep
        dispSeconds(time_left);
        strikeLEDOff(); // turn off strike LEDs
    }

    // sprintf(char_buffer, "Num Strikes:%d, Per. Pres:%d%d%d%d%d%d, Per. Complete:%d%d%d%d%d%d", num_strikes,
    //     peripheral_present[0], peripheral_present[1], peripheral_present[2], peripheral_present[3], peripheral_present[4], peripheral_present[5],
    //     peripheral_complete[0], peripheral_complete[1], peripheral_complete[2], peripheral_complete[3], peripheral_complete[4], peripheral_complete[5]);
    // U1_puts(char_buffer);
    // U1_putc('\r');
    // U1_putc('\n');
    // U1_flush_tx_buffer();
}

void end_fail(void) {
    if (state != last_state) {  // if we are entering the state, do initialization stuff
        last_state = state;
        // start timer again to blink red LEDs
        IFS0bits.T1IF = 0; //lower interrupt flag
        TMR1 = 0;          // reset timer register
        T1CONbits.TON = 1; // enable 1 second timer

        OC1RS = PWM_PERIOD_40;
        OC1R = OC1RS>>1; // start beep

        STRIKE1_RLED = ON; delay_by_nop(1); // Turn on strike LEDs red
        STRIKE2_RLED = ON; delay_by_nop(1);
        STRIKE3_RLED = ON; delay_by_nop(1);
    }

    // Perform state tasks
    if (IFS0bits.T1IF == 1) {
        IFS0bits.T1IF = 0;  // if it's been a second, lower the counter and show it
        // Blink RLEDs
        STRIKE1_RLED = !STRIKE1_RLED; delay_by_nop(1);
        STRIKE2_RLED = !STRIKE2_RLED; delay_by_nop(1);
        STRIKE3_RLED = !STRIKE3_RLED; delay_by_nop(1);
    }

    // Check for state transitions

    /* NO TRANSITIONS */

    // if we are leaving the state, do clean up stuff
    if (state != last_state) {
        T1CONbits.TON = 0;
        OC1R = 0; // stop beep
        STRIKE1_RLED = OFF; delay_by_nop(1);
        STRIKE2_RLED = OFF; delay_by_nop(1); // turn off strike LEDs
        STRIKE3_RLED = OFF; delay_by_nop(1);
    }
}

void end_win(void) {
    if (state != last_state) {  // if we are entering the state, do initialization stuff
        last_state = state;
        // start timer again to blink red LEDs
        IFS0bits.T1IF = 0; //lower interrupt flag
        IEC0bits.T1IE = 0;
        TMR1 = 0;          // reset timer register
        T1CONbits.TON = 1; // enable 1 second timer
        STRIKE1_GLED = ON; delay_by_nop(1); // Turn on strike LEDs red
        STRIKE2_GLED = ON; delay_by_nop(1);
        STRIKE3_GLED = ON; delay_by_nop(1);
    }

    // Perform state tasks
    if (IFS0bits.T1IF == 1) {
        IFS0bits.T1IF = 0;  // if it's been a second, lower the counter and show it
        // Blink GLEDs
        STRIKE1_GLED = !STRIKE1_GLED; delay_by_nop(1);
        STRIKE2_GLED = !STRIKE2_GLED; delay_by_nop(1);
        STRIKE3_GLED = !STRIKE3_GLED; delay_by_nop(1);
    }

    // Check for state transitions

    /* NO TRANSITIONS */

    // if we are leaving the state, do clean up stuff
    if (state != last_state) {
        T1CONbits.TON = 0;
        STRIKE1_GLED = OFF; delay_by_nop(1);
        STRIKE2_GLED = OFF; delay_by_nop(1); // turn off strike LEDs
        STRIKE3_GLED = OFF; delay_by_nop(1);
    }
}

// HELPER FUNCTIONS ************************************************************

// Display a seconds variable in minutes and seconds on the seven segment
void dispSeconds(uint16_t seconds) {
    // Turn seconds into minutes and seconds
    uint16_t minutes = seconds / 60;
    uint8_t displaySeconds = seconds % 60;
    sevseg_writeDigitNum(&matrix, 0, minutes / 10, 0);
    sevseg_writeDigitNum(&matrix, 1, minutes % 10, 0);
    sevseg_drawColon(&matrix, 1); // Times are supposed to have a colon I guess
    sevseg_writeDigitNum(&matrix, 3, (displaySeconds / 10) % 10, 0);
    sevseg_writeDigitNum(&matrix, 4, displaySeconds % 10, 0);

    led_writeDisplay((_ADAFRUIT_LED*)&matrix.super); //Don't forget to actually write the data!
}

// Draw 1234 on the seven segment
void drawOnce(void) {
    sevseg_writeDigitNum(&matrix, 0, 1, 0);
    sevseg_writeDigitNum(&matrix, 1, 2, 0);
    sevseg_writeDigitNum(&matrix, 3, 3, 0);
    sevseg_writeDigitNum(&matrix, 4, 4, 0);
    led_writeDisplay((_ADAFRUIT_LED*)&matrix.super);
}
