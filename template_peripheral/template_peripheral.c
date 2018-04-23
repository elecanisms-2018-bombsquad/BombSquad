#include "elecanisms.h"
#include "peripheral_core.h"
#include "i2c_address_space.h"

#define MODULE_LED_RED      D0
#define MODULE_LED_GREEN    D1

typedef void (*STATE_HANDLER_T)(void);

// forward declaration of module modes
void setup(void);
void run(void);
void solved(void);
void end_win(void);
void end_fail(void);

STATE_HANDLER_T state, last_state;

void ledoff(void) {
    LED1 = 0; delay_by_nop(1);
    LED2 = 0; delay_by_nop(1);
    LED3 = 0; delay_by_nop(1);
    D0 = OFF;
}

int16_t main(void) {
    init_elecanisms();
    i2c2_init(157);                      // Initializes I2C on I2C2
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
        MODULE_LED_GREEN = ON;
        // setup state here
    }

    // Perform state tasks

    // Check for state transitions
    // if ((start_flag == 1) || (SW2 == 0)){
    //     state = run;
    // }
    complete_flag = 0;
    state = run;

    // State Cleanup
    if (state != last_state) {
        // cleanup state here
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
    }

    // Perform state tasks


    // Check for state transitions
    if (win_flag == 1) {
        state = end_win;
    } else if (lose_flag == 1) {
        state = end_fail;
    } else if (SW2 == 0 ) {
        state = solved;
    }

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
        last_state = state;
        LED3 = ON;
        complete_flag = 1;
        MODULE_LED_GREEN = ON;
        // setup state here
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
        // setup state here
    }

    // Perform state tasks

    // Check for state transitions
    if (start_flag == 1) {
        state = run;
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
        last_state = state;
        MODULE_LED_RED = ON;
    }

    // Perform state tasks

    // Check for state transitions
    if (start_flag == 1) {
        state = run;
    }

    // State Cleanup
    if (state != last_state) {
        // cleanup state here
        MODULE_LED_RED = OFF;
    }
}
