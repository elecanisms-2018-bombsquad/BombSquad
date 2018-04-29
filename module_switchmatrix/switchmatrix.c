#include <stdio.h>

#include "elecanisms.h"
#include "peripheral_core.h"
#include "i2c_address_space.h"
#include "ajuart.h"
#include "adafruit_led.h"

char buffer[128];

#define switch1  0b0000000001   // wait / okay
#define switch2  0b0000000010   // uh huh / uh uh
#define switch3  0b0000000100   // V / other V
#define switch4  0b0000001000   // blank / _
#define switch5  0b0000010000   // down / up
#define switch6  0b0000100000   // star / *
#define switch7  0b0001000000   // done / next
#define switch8  0b0010000000   // wait / no
// #define switch9  0b0100000000

uint16_t switches = 0;

#define MODULE_LED_RED      D0
#define MODULE_LED_GREEN    D1

typedef void (*STATE_HANDLER_T)(void); // frame of game states
void setup(void);// forward declaration of module modes
void run(void);
void solved(void);
void end_win(void);
void end_fail(void);
STATE_HANDLER_T state, last_state;

typedef void (*GAME_STATE)(void);  // the sattes the games goes through
void first_gamestate(void);
void second_gamestate(void);
void third_gamestate(void);
GAME_STATE num_gamestate;

uint8_t done_striked;

void ledoff(void); // define function
void pinsetup(void);
void checkSwitches(void);

// volatile uint16_t switches = 0;


// void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
//     // LED1 = 1;
//     IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
// }



int16_t main(void) {
    init_elecanisms();
    init_ajuart();

    D0_DIR = OUT;
    D1_DIR = OUT;

    i2c2_init(157);                      // Initializes I2C on I2C2
    I2C2ADD = TEST_PERIPHERAL_ADDR>>1;   // Set the device address (7-bit register)
    I2C2MSK = 0;                         // Set mask to 0 (only this address matters)
    _SI2C2IE = 1;                        // Enable i2c slave interrupt
    //
    // T1CON = 0x0020;         // set Timer1cd .. period to 10 ms for debounce
    // PR1 = 0x2710;           // prescaler 16, match value 10000
    // TMR1 = 0;               // set Timer1 count to 0
    // IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    // IEC0bits.T1IE = 1;      // enable Timer1 interrupt
    // T1CONbits.TON = 1;      // turn on Timer1

    pinsetup();

    state = setup;

    while (1) {

        state();
    }
} // end of main


// STATE MACHINE FUNCTIONS /////////////////////////////////////////////////////


void setup(void) { // Waits for master module to start the game
    // State Setup
    U1_putc(100);
    U1_putc(switches);
    // U1_putc('\r');
    U1_putc('\n');
    U1_flush_tx_buffer();
    delay_by_nop(30000);


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

    // below lines need to be uncommented so we go to the run state at the right time
    // if ((start_flag == 1) || (SW2 == 0)){
        state = run;
    // }

    // State Cleanup
    if (state != last_state) {
        MODULE_LED_RED = OFF; delay_by_nop(1);
        MODULE_LED_GREEN = OFF;
    }
}

void run(void) { // Plays the game
    // State Setup
    if (state != last_state) {
        last_state = state;
        MODULE_LED_RED = ON;
        done_striked = 0;
    }

    // Perform state tasks

    // switches
    checkSwitches();


    sprintf(buffer, "switches is %x", switches);
    U1_putc('\r');
    U1_putc('\n');
    U1_puts((uint8_t*)buffer);
    U1_flush_tx_buffer();
    delay_by_nop(1500);





// for when it is done game coding
    // Check for state transitions
    // if (win_flag == 1) {
    //     state = end_win;
    // } else if (lose_flag == 1) {
    //     state = end_fail;
    // }
    // if (SW2 == 0 ) {
    //     state = solved;
    // }
    // if (SW1 == 0) {
    //     if (!done_striked) {
    //         num_strikes++;
    //         done_striked = 1;
    //     }
    // }
    // if (SW3 == 0) {
    //     if (!done_striked) {
    //         num_strikes+=3;
    //         done_striked = 1;
    //     }
    // }


    // State Cleanup
    if (state != last_state) {
        MODULE_LED_RED = OFF;
    }
}

void solved(void) { // The puzzle on this module is finished
    // State Setup
    if (state != last_state) {
        last_state = state;
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
        complete_flag = 0;
        MODULE_LED_GREEN = OFF;
    }
}

void end_win(void) { // The master module said the game was won
    // // State Setup
    // if (state != last_state) {
    //     last_state = state;
    //     MODULE_LED_GREEN = ON;
    //
    //     T1CON = 0x0030;         // set Timer1 period to 0.5s
    //     PR1 = 0x7A11;
    //
    //     TMR1 = 0;               // set Timer1 count to 0
    //     IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    //     T1CONbits.TON = 1;      // turn on Timer1
    //     // setup state here
    // }
    //
    // // Perform state tasks
    // // if (IFS0bits.T1IF == 1) {
    // //     IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    // //     MODULE_LED_GREEN = !MODULE_LED_GREEN;           // toggle LED
    // // }
    //
    //
    // // State Cleanup
    // if (state != last_state) {
    //     MODULE_LED_GREEN = OFF;
    // }
}

void end_fail(void) { // The master module said the game was lost
    // State Setup
    // if (state != last_state) {
    //     last_state = state;
    //     MODULE_LED_RED = ON;
    //
    //     T1CON = 0x0030;         // set Timer1 period to 0.5s
    //     PR1 = 0x7A11;
    //
    //     TMR1 = 0;               // set Timer1 count to 0
    //     IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    //     T1CONbits.TON = 1;      // turn on Timer1
    // }
    //
    // // Perform state tasks
    // if (IFS0bits.T1IF == 1) {
    //     IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    //     MODULE_LED_RED = !MODULE_LED_RED;           // toggle LED
    // }
    //
    //
    // // State Cleanup
    // if (state != last_state) {
    //     MODULE_LED_RED = OFF;
    //     T1CONbits.TON = 0;      // turn off Timer1
    // }
}

void ledoff(void) {
    LED1 = 0; delay_by_nop(1);
    LED2 = 0; delay_by_nop(1);
    LED3 = 0; delay_by_nop(1);
    D0 = OFF;
}

void pinsetup(void){
    D4_DIR = 1;  __asm__("nop");
    D5_DIR = 1;  __asm__("nop");
    D6_DIR = 1;  __asm__("nop");
    D7_DIR = 1;  __asm__("nop");
    D8_DIR = 1;  __asm__("nop");
    D9_DIR = 1;  __asm__("nop");
    D10_DIR = 1; __asm__("nop");
    D12_DIR = 1; __asm__("nop");
    D13_DIR = 1; __asm__("nop");

    D4_PUE = 1;  __asm__("nop");
    D5_PUE = 1;  __asm__("nop");
    D6_PUE = 1;  __asm__("nop");
    D7_PUE = 1;  __asm__("nop");
    D8_PUE = 1;  __asm__("nop");
    D9_PUE = 1;  __asm__("nop");
    D10_PUE = 1; __asm__("nop");
    D12_PUE = 1; __asm__("nop");
    D13_PUE = 1; __asm__("nop");
}

void checkSwitches(void){
    if (D4 == 0)  {switches = (switches |  switch1); delay_by_nop(10); }
    if (D4 == 1)  {switches = (switches & ~switch1); delay_by_nop(10); }
    if (D5 == 0)  {switches = (switches |  switch2); delay_by_nop(10); }
    if (D5 == 1)  {switches = (switches & ~switch2); delay_by_nop(10); }
    if (D6 == 0)  {switches = (switches |  switch3); delay_by_nop(10); }
    if (D6 == 1)  {switches = (switches & ~switch3); delay_by_nop(10); }
    if (D7 == 0)  {switches = (switches |  switch4); delay_by_nop(10); }
    if (D7 == 1)  {switches = (switches & ~switch4); delay_by_nop(10); }
    if (D8 == 0)  {switches = (switches |  switch5); delay_by_nop(10); }
    if (D8 == 1)  {switches = (switches & ~switch5); delay_by_nop(10); }
    if (D9 == 0)  {switches = (switches |  switch6); delay_by_nop(10); }
    if (D9 == 1)  {switches = (switches & ~switch6); delay_by_nop(10); }
    if (D10 == 0) {switches = (switches |  switch7); delay_by_nop(10); }
    if (D10 == 1) {switches = (switches & ~switch7); delay_by_nop(10); }
    if (D13 == 0) {switches = (switches |  switch8); delay_by_nop(10); }
    if (D13 == 1) {switches = (switches & ~switch8); delay_by_nop(10); }
    // if (D13 == 0) {switches = (switches |  switch9); delay_by_nop(10); }
    // if (D13 == 1) {switches = (switches & ~switch9); delay_by_nop(10); }
}
