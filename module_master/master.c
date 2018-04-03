#include <p24FJ128GB206.h>
#include <stdint.h>

#include "common.h" // common.h needed for _init_clock, not sure if needed
#include "elecanisms.h"
#include "i2c_reg.h"
#include "adafruit_led.h"

#define STRIKE1_RLED      D0
#define STRIKE1_GLED      D1
#define STRIKE2_RLED      D2
#define STRIKE2_GLED      D3
#define STRIKE3_RLED      D4
#define STRIKE3_GLED      D5
#define BUTTON_LED        D6
#define BUTTON_LED_LOW    D7

#define max_time 10

_7SEGMENT matrix;

const uint8_t target_addr = 0xE0;
uint16_t time_left;

// Forward declarations of functions to avoid a header file :/
void idle(void);
void active(void);
void end(void);
void dispSeconds(uint16_t seconds);

typedef void (*STATE_HANDLER_T)(void);
STATE_HANDLER_T state, last_state;

// STATE MACHINE FUNCTIONS *****************************************************

void idle(void) {
    if (state != last_state) {  // if we are entering the state, do initialization stuff
        last_state = state;
        dispSeconds(max_time); // Start by showing full time
        BUTTON_LED = ON;
    }

    // Perform state tasks

    // Check for state transitions
    if (D8 == 0) { // D8 is pulled-up, if button is pressed it pulls it down
        state = active;
    }

    // if we are leaving the state, do clean up stuff
    if (state != last_state) {
        BUTTON_LED = OFF;
    }
}

void active(void) {
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
    }

    // Perform state tasks
    if (IFS0bits.T1IF == 1) {
        IFS0bits.T1IF = 0;  // if it's been a second, lower the counter and show it
        time_left--;
        dispSeconds(time_left);
    }

    // Check for state transitions
    if (SW2 == 0) {
        state = idle;           // SW2 resets to idle
    } else if (time_left == 0) {
        state = end;            // if we're out of time, go to end
    }

    // if we are leaving the state, do clean up stuff
    if (state != last_state) {
        T1CONbits.TON = 0;
        STRIKE1_GLED = OFF; // Turn off strike LEDs.
        delay_by_nop(1);
        STRIKE2_GLED = OFF;
        delay_by_nop(1);
        STRIKE3_GLED = OFF;
    }
}

void end(void) {
    if (state != last_state) {  // if we are entering the state, do initialization stuff
        last_state = state;
        // start timer again to blink red LEDs
        IFS0bits.T1IF = 0; //lower interrupt flag
        TMR1 = 0;          // reset timer register
        T1CONbits.TON = 1; // enable 1 second timer
        STRIKE1_RLED = ON; // Turn on strike LEDs red
        STRIKE2_RLED = ON;
        STRIKE3_RLED = ON;
    }

    // Perform state tasks
    if (IFS0bits.T1IF == 1) {
        IFS0bits.T1IF = 0;  // if it's been a second, lower the counter and show it
        // Blink RLEDs
        STRIKE1_RLED = !STRIKE1_RLED;
        STRIKE2_RLED = !STRIKE2_RLED;
        STRIKE3_RLED = !STRIKE3_RLED;
    }

    // Check for state transitions

    /* NO TRANSITIONS */

    // if we are leaving the state, do clean up stuff
    if (state != last_state) {
        T1CONbits.TON = 0;
        STRIKE1_RLED = OFF;
        STRIKE2_RLED = OFF; // turn off strike LEDs
        STRIKE3_RLED = OFF;
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

// MAIN FUNCTION ***************************************************************

int16_t main(void) {
    init_elecanisms();
    init_clock(); // not sure if this does anything
    // Initializes I2C on I2C3
    i2c_init(1e3);
    led_begin((_ADAFRUIT_LED*)&matrix.super, target_addr); // Set up the HT16K33 and start the oscillator

    time_left = max_time; // Set up time left to be max time

    // Setup pins as output
    D0_DIR = 0;
    D1_DIR = 0;
    D2_DIR = 0;
    D3_DIR = 0;
    D4_DIR = 0;
    D5_DIR = 0;
    D6_DIR = 0;
    D7_DIR = 0;
    D9_DIR = 0;


    // Init button low
    BUTTON_LED = 0;
    BUTTON_LED_LOW = 0;

    // Setup D8 as input
    D8_DIR = 1;
    // Set up pull-up resistor on button D8
    CNPU4bits.CN54PUE = 1;
    // Set D9 as pull-down for button
    D9 = 0;

    T1CON = 0x0030;         // set Timer1 period to 1s
    PR1 = 0xF422;
    TMR1 = 0;               // set Timer1 count to 0
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag

    state = idle;           // Initialize state to idle
    last_state = (STATE_HANDLER_T)NULL;

    while (1) {
        state();
    }

}
