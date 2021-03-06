#include "elecanisms.h"
#include "peripheral_core.h"
#include "i2c_address_space.h"
#include "ajuart.h"

#define MODULE_LED_RED      D0
#define MODULE_LED_GREEN    D1

typedef void (*STATE_HANDLER_T)(void);

// forward declaration of module modes
void setup(void);
void run(void);
void solved(void);
void end_win(void);
void end_fail(void);

uint8_t has_struck;

STATE_HANDLER_T state, last_state;

void ledoff(void) {
    LED1 = 0; delay_by_nop(1);
    LED2 = 0; delay_by_nop(1);
    LED3 = 0; delay_by_nop(1);
    D0 = OFF;
}

int16_t main(void) {
    init_elecanisms();
    i2c2_init(157);
    init_ajuart();                      // Initializes I2C on I2C2
    I2C2ADD = TEST_PERIPHERAL_ADDR>>1;   // Set the device address (7-bit register)
    I2C2MSK = 0;                         // Set mask to 0 (only this address matters)
    _SI2C2IE = 1;                        // Enable i2c slave interrupt

    D0_DIR = OUT;
    D1_DIR = OUT;

    state = setup;

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
    if (SW3 == 0) {
        if (!has_struck) {
            num_strikes+=3;
            has_struck = 1;
        }
    }
    U1_putc(num_strikes);
    U1_putc('\r');
    U1_putc('\n');
    U1_flush_tx_buffer();

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
