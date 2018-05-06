#include <stdio.h>
#include <string.h>
#include "elecanisms.h"
#include "adafruit_led.h"
#include "peripheral_core.h"
#include "i2c_address_space.h"
#include "ajuart.h"
#include "morse.h"

#define MODULE_LED_RED      D0
#define MODULE_LED_GREEN    D1

#define MORSE_PIN           D13
#define MORSE_MS            300000

typedef void (*STATE_HANDLER_T)(void);

// forward declaration of module modes
void setup(void);
void run(void);
void solved(void);
void end_win(void);
void end_fail(void);

uint8_t has_struck;

uint16_t rand_val;
uint8_t morse_idx;
uint8_t morse_counter;
uint8_t morse_table_idx;
uint16_t target_freq;
char *morse;
char char_buffer[128];

uint16_t new_freq_mhz = 0;
uint16_t prev_freq_mhz = 0;
uint16_t freq_mhz = 0;
uint8_t tx_pressed = 0;

STATE_HANDLER_T state, last_state;

_7SEGMENT matrix;
const uint8_t matrix_addr = 0xE0;

void ledoff(void) {
    LED1 = 0; delay_by_nop(1);
    LED2 = 0; delay_by_nop(1);
    LED3 = 0; delay_by_nop(1);
    D0 = OFF;
}

int16_t main(void) {
    init_elecanisms();
    // Initializes I2C on I2C3
    i2c_init(1e3);
    I2Cpoll(matrix_addr);
    led_begin((_ADAFRUIT_LED*)&matrix.super, matrix_addr); // Set up the HT16K33 and start the oscillator

    i2c2_init(157);
    init_ajuart();                      // Initializes I2C on I2C2
    I2C2ADD = MODULE_MORSE_ADDR>>1;   // Set the device address (7-bit register)
    I2C2MSK = 0;                         // Set mask to 0 (only this address matters)
    _SI2C2IE = 1;                        // Enable i2c slave interrupt


    /* Set up pins and Change Notification interrupts */
    D0_DIR = OUT;
    D1_DIR = OUT;
    D13_DIR = OUT;
    D8_DIR = IN;
    D8_PUE = 1;
    D8_CNEN = 1;

    IFS1bits.CNIF = 0; // lower CN interrupt flag
    IEC1bits.CNIE = 1; // Enable CN interrupt module

    state = setup;

    /* Pick morse word */
    rand_val = read_analog(A5_AN); // Set up the seed
    // Add more random noise
    uint8_t i, j;
    for (i=0; i<20; i++) {
        for (j=0; j<read_analog(A5_AN); j++) {
            rand_next();
            delay_by_nop(read_analog(A5_AN));
        }
    }
    morse_table_idx = rand_val%14; // Pick index in morse word table
    char _morse[32] = ""; // temp array
    for (i=0; i<32; i++){
        _morse[i] = morse_table[morse_table_idx][i]; // Copy morse table line
    }
    morse = _morse; // Set morse pointer to temp
    target_freq = freq_table[morse_table_idx];

    U1_puts(morse);
    U1_putc('\r');
    U1_putc('\n');
    U1_flush_tx_buffer();

    T2CON = 0x0020;         // set Timer2 period to 10 ms for analog debounce
    PR2 = 0x1710;           // prescaler 16, match value 10000
    // T1CON = 0x0030; // PR1 = 0x3D08;        // set Timer2 period to 0.25s, prescaler 256 match 15624
    TMR2 = 0;               // set Timer2 count to 0
    IFS0bits.T2IF = 0;      // lower Timer2 interrupt flag
    IEC0bits.T2IE = 1;      // enable Timer2 interrupt
    T2CONbits.TON = 1;      // turn on Timer2

    T4CON = 0x0020;         // set Timer4 period to 10 ms for tx switch debounce
    PR4 = 0x1710;           // prescaler 16, match value 10000
    // T1CON = 0x0030; // PR1 = 0x3D08;        // set Timer2 period to 0.25s, prescaler 256 match 15624
    TMR4 = 0;               // set Timer4 count to 0
    IFS1bits.T4IF = 0;      // lower Timer4 interrupt flag
    IEC1bits.T4IE = 1;      // enable Timer4 interrupt
    T4CONbits.TON = 1;      // turn on Timer4


    while (1) {
        state();
    }
} // end of main

// STATE MACHINE FUNCTIONS /////////////////////////////////////////////////////


void setup(void) { // Waits for master module to start the game
    // State Setup
    if (state != last_state) {
        last_state = state;
        MODULE_LED_GREEN = ON; delay_by_nop(1);
        MODULE_LED_RED = ON;
        complete_flag = 0;
        num_strikes = 0;
        error_code = 0;
        // setup state here
    }

    // Perform state tasks

    //Check for state transitions
    if ((start_flag == 1) || (SW2 == 0)){
        state = run;
    }

    // State Cleanup
    if (state != last_state) {
        // cleanup state here
        MODULE_LED_RED = OFF; delay_by_nop(1);
        MODULE_LED_GREEN = OFF;
    }
}

void run(void) { // Plays the game
    // State Setup
    if (state != last_state) {
        last_state = state;
        // setup state here
        LED1 = ON; delay_by_nop(1);
        MODULE_LED_RED = ON;
        has_struck = 0;

        T1CON = 0x0030;         // set Timer1 period to 0.25s
        PR1 = 0x3D08;

        TMR1 = 0;               // set Timer1 count to 0
        IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
        T1CONbits.TON = 1;      // turn on Timer1

        morse_idx = 0;
        morse_counter = 0;
        tx_pressed = 0;
    }

    // Perform state tasks


    // Check for state transitions
    if (win_flag == 1) {
        state = end_win;
    } else if (lose_flag == 1) {
        state = end_fail;
    }
    if (SW2 == 0 ) {
        state = solved;
    }
    if (SW1 == 0) {
        if (!has_struck) {
            num_strikes++;
            has_struck = 1;
        }
    }
    if (tx_pressed) {
        tx_pressed = 0;
        if (freq_mhz > (target_freq - 1) && freq_mhz < (target_freq + 1)) { //Give leeway of 1
            state = solved;
        } else {
            num_strikes++;
        }
    }

    // /* Filter input from frequency slider */
    // uint16_t analog_reading = read_analog(A0_AN);
    // uint16_t analog_filtered = (analog_filtered * 0.1) + (analog_reading * 0.9);
    // uint16_t freq_mhz = 3500 + ((analog_filtered + 50)/10);

    disp_mhz(freq_mhz);

    // sprintf(char_buffer, "MHz value: %d", freq_mhz);
    // U1_puts(char_buffer);
    // U1_putc('\r');
    // U1_putc('\n');
    // U1_flush_tx_buffer();


    /* handle morse blinks */

    if (_T1IF == 1) {
        _T1IF = 0;
        doMorse(morse);
    }
    // doMorse(morse);

    delay_by_nop(300);

    // State Cleanup
    if (state != last_state) {
        // cleanup state here
        LED1=OFF; delay_by_nop(1);
        MODULE_LED_RED = OFF;
    }
}

void solved(void) { // The puzzle on this module is finished
    // State Setup
    if (state != last_state) {
        // setup state here
        last_state = state;
        LED3 = ON;
        complete_flag = 1;
        MODULE_LED_GREEN = ON;
    }

    // Perform state tasks


    // Check for state transitions
    if (win_flag == 1) {
        state = end_win;
    } else if (lose_flag == 1) {
        state = end_fail;
    }

    // State Cleanup
    if (state != last_state) {
        // cleanup state here
        LED3 = OFF;
        complete_flag = 0;
        MODULE_LED_GREEN = OFF;
    }
}

void end_win(void) { // The master module said the game was won
    // State Setup
    if (state != last_state) {
        last_state = state;
        MODULE_LED_GREEN = ON;

        T1CON = 0x0030;         // set Timer1 period to 0.5s
        PR1 = 0x7A11;

        TMR1 = 0;               // set Timer1 count to 0
        IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
        T1CONbits.TON = 1;      // turn on Timer1
        // setup state here
    }

    // Perform state tasks
    if (IFS0bits.T1IF == 1) {
        IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
        MODULE_LED_GREEN = !MODULE_LED_GREEN;           // toggle LED
    }


    // State Cleanup
    if (state != last_state) {
        // cleanup state here
        MODULE_LED_GREEN = OFF;
    }
}

void end_fail(void) { // The master module said the game was lost
    // State Setup
    if (state != last_state) {
        // setup state here
        last_state = state;
        MODULE_LED_RED = ON;

        T1CON = 0x0030;         // set Timer1 period to 0.5s
        PR1 = 0x7A11;

        TMR1 = 0;               // set Timer1 count to 0
        IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
        T1CONbits.TON = 1;      // turn on Timer1
    }

    // Perform state tasks
    if (IFS0bits.T1IF == 1) {
        IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
        MODULE_LED_RED = !MODULE_LED_RED;           // toggle LED
    }


    // State Cleanup
    if (state != last_state) {
        // cleanup state here
        MODULE_LED_RED = OFF;
        T1CONbits.TON = 0;      // turn off Timer1
    }
}

// ISRs ////////////////////////////////////////////////////////////////////////

void __attribute__((interrupt, auto_psv)) _T2Interrupt(void) {
    IFS0bits.T2IF = 0;      // lower Timer2 interrupt flag
    prev_freq_mhz = new_freq_mhz;
    new_freq_mhz = freq_mhz = 3500 + ((read_analog(A0_AN) + 50)/10);
    if (new_freq_mhz == prev_freq_mhz) {
        freq_mhz = new_freq_mhz;
    }
}

void __attribute__((interrupt, auto_psv)) _CNInterrupt(void) {
    LED1 = ON;
    IFS1bits.CNIF = 0;      // lower INT3 interrupt flag
    TMR4 = 0;                 // reset debounce Timer4
    IFS1bits.T4IF = 0;        // lower Timer4 Interrupt flag
    T4CONbits.TON = 1;        // start Timer4
}

void __attribute__((interrupt, auto_psv)) _T4Interrupt(void) {
    LED1 = OFF;
    IFS1bits.T4IF = 0;
    T4CONbits.TON = 0;
    if (D8 == 0) {
        tx_pressed = 1;
    }
}

// HELPER FUNCTIONS ////////////////////////////////////////////////////////////

void disp_mhz(uint16_t number) {
    uint16_t num_new;
    uint8_t thousands, hundreds, tens, ones;
    thousands = number / 1000 ;
    num_new = number - ( thousands * 1000);
    hundreds = num_new  / 100 ;
    num_new = num_new - (hundreds * 100);
    tens = num_new / 10 ;
    num_new = num_new - (tens * 10) ;
    ones = num_new ;

    sevseg_writeDigitNum(&matrix, 0, thousands, 1);
    sevseg_writeDigitNum(&matrix, 1, hundreds, 0);
    sevseg_writeDigitNum(&matrix, 3, tens, 0);
    sevseg_writeDigitNum(&matrix, 4, ones, 0);
    led_writeDisplay((_ADAFRUIT_LED*)&matrix.super); //Don't forget to actually write the data!
}

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

void doMorse(char* morse_str) {
    switch (morse_str[morse_idx]) {
        case '.': // On for 1 period, off for 1 period
            if (MORSE_PIN == OFF) {
                MORSE_PIN = ON;
            } else { // MORSE_PIN == ON
                MORSE_PIN = OFF;
                morse_idx ++; // advance to next char
            }
        break;
        case '-': // On for 3 periods, off for 1 period
            if (MORSE_PIN == OFF) {
                MORSE_PIN = ON;
            } else if (morse_counter < 3) {
                morse_counter ++;
            } else { // ON, ctr == 3
                MORSE_PIN = OFF;
                morse_counter = 0;
                morse_idx ++;
            }
        break;
        case ' ': // Off for 3 periods more
            MORSE_PIN = OFF;
            if (morse_counter < 3) {
                morse_counter ++;
            } else {
                morse_counter = 0;
                morse_idx++;
            }
        break;
        default: // Word break, delay for 16 periods then restart
            MORSE_PIN = OFF;
            if (morse_counter < 16) {
                morse_counter ++;
            } else {
                morse_counter = 0;
                morse_idx = 0;
            }
        break;
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

    // U1_putc(thousands); U1_putc(hundreds); U1_putc(tens); U1_putc(ones);
    // U1_putc('\r'); U1_putc('\n'); U1_flush_tx_buffer();

    sevseg_writeDigitNum(&matrix, 0, 17, 0);
    sevseg_writeDigitNum(&matrix, 4, 17, 0);
    sevseg_writeDigitNum(&matrix, 1, tens, 0);
    sevseg_writeDigitNum(&matrix, 3, ones, 0);
    led_writeDisplay((_ADAFRUIT_LED*)&matrix.super); //Don't forget to actually write the data!
}
