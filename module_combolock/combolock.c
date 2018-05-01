#include "elecanisms.h"
#include "peripheral_core.h"
#include "i2c_address_space.h"
#include "ajuart.h"
#include "adafruit_led.h"

#define MODULE_LED_RED      D0
#define MODULE_LED_GREEN    D1

typedef void (*STATE_HANDLER_T)(void);  // main frame of game
void setup(void);   // forward declaration of module modes
void run(void);
void solved(void);
void end_win(void);
void end_fail(void);
STATE_HANDLER_T state, last_state;

/////////////////////////////////

typedef void (*GAME_STATE)(void);  // the sattes the games goes through
void firstnum(void);
void secondnum(void);
void thirdnum(void);
GAME_STATE combo_num;


void ledoff(void);
void dispNumber(uint16_t number);

uint8_t current_display = 0;
uint8_t previous_display = 0;

uint16_t new_debounce_reading = 0;
uint16_t prev_debounce_reading = 0;

uint16_t eval_reading = 0;

uint8_t combo1, combo2, combo3;
uint8_t counter = 0;
uint8_t tempval = 0;

_7SEGMENT matrix;

const uint8_t adafruit_display_addr = 0xE0;


void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
    // LED1 = 1;
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    prev_debounce_reading = new_debounce_reading;
    new_debounce_reading = read_analog(A0_AN) >> 5 ;
    if (new_debounce_reading == prev_debounce_reading) {
        eval_reading = new_debounce_reading;
        // LED3 = 1;
    }
    if(current_display != eval_reading){
        previous_display = current_display;
        current_display = eval_reading;
    }
}

int16_t main(void) {
    init_elecanisms();

    i2c2_init(157);                      // Initializes I2C on I2C2
    I2C2ADD = MODULE_CODEWORD_ADDR>>1;   // Set the device address (7-bit register)
    I2C2MSK = 0;                         // Set mask to 0 (only this address matters)
    _SI2C2IE = 1;                        // Enable i2c slave interrupt

    D0_DIR = OUT;
    D1_DIR = OUT;

    init_ajuart();
    i2c_init(1e3);      // initializes I2C3 for screen thing
    led_begin((_ADAFRUIT_LED*)&matrix.super, adafruit_display_addr); // Set up the HT16K33 and start the oscillator

    T1CON = 0x0020;         // set Timer1cd .. period to 10 ms for debounce
    PR1 = 0x1710;           // prescaler 16, match value 10000
    // T1CON = 0x0030; // PR1 = 0x3D08;        // set Timer1 period to 0.25s, prescaler 256 match 15624
    TMR1 = 0;               // set Timer1 count to 0
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    IEC0bits.T1IE = 1;      // enable Timer1 interrupt
    T1CONbits.TON = 1;      // turn on Timer1

    state = setup;

    while (1) {
        dispNumber(eval_reading);
        state();
        // LED2 = 1;
    } // end of while
} // end of main




///////////////////////////////////////////////////////////////////////////
///////////////// STATE MACHINE FUNCTIONS /////////////////////////////////
///////////////////////////////////////////////////////////////////////////



void firstnum(void){ // turn right 3 times around , right is numbers going down
    ledoff();
    LED1 = 1;
    int8_t delta = previous_display - current_display ;

    if ( (delta < -1) && (delta > -20) ) { //catch it if you go backwards
        counter = 0;
    }

    if (current_display == combo1 )  { // set a temp if you get the number to turn past
        tempval = 1;
    }
    if (previous_display == combo1 && tempval == 1) { // if you move on to a new number after that
        counter = counter + 1;
        tempval = 0;
    }
    if(counter > 1 && tempval == 1){ // go to next direction
        combo_num = secondnum;      // cleanup things here
        counter = 0;
        tempval = 0;
        previous_display = current_display; // so you don't think you went backwards as soon as you enter the state
    }
    // U1_putc(1);
    // U1_putc(counter);
    // U1_putc(tempval);
    // U1_putc('\r');
    // U1_putc('\n');
    // U1_flush_tx_buffer();
    // delay_by_nop(15);

}

void secondnum(void){ // turn left 1 extra time around , left us numbers going up
    ledoff();
    LED2 = 1;

    int8_t delta = previous_display - current_display ;

    if (current_display == combo2 ){ // set a temp if you get the number to turn past
        tempval = 1;
    }

    if (previous_display == combo2 && tempval == 1) { // if you move on to a new number after that
        counter = counter + 1;
        tempval = 0;
    }
    if(counter == 1 && tempval == 1){ // go to next direction
        combo_num = thirdnum;      // cleanup things here
        counter = 0;
        tempval = 0;
        previous_display = current_display; // so you don't think you went backwards as soon as you enter the state

    }
    if ( (delta > 1) && (delta < 20) ) {  //catch it if you go backwards
        counter = 0;
        tempval = 0;
        combo_num = firstnum;
    }

    // U1_putc(2);
    // U1_putc(counter);
    // U1_putc(tempval);
    // U1_putc('\r');
    // U1_putc('\n');
    // U1_flush_tx_buffer();
    // delay_by_nop(15);
}

void thirdnum(void){
    ledoff();
    LED3 = 1;
    int8_t delta = previous_display - current_display ;
    if (current_display == combo3 ) {
        combo_num = firstnum;
        counter = 0;
        state = solved;
    }
    if ( (delta < -1) && (delta > -20) ) {  //catch it if you go backwards
        combo_num = firstnum;
        counter = 0;
    }

    // U1_putc(3);
    // U1_putc(counter);
    // U1_putc(tempval);
    // U1_putc('\r');
    // U1_putc('\n');
    // U1_flush_tx_buffer();
    // delay_by_nop(15);
}

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
    // usually this will determine what the combo values are based on the serial number
    // if (serial_number == 1) { combo1 = 5; combo2 = 10; combo3 = 15; }
    // if (serial_number == 2) { combo1 = 13; combo2 = 7; combo3 = 2; }
    // if (serial_number == 3) { combo1 = 17; combo2 = 8; combo3 = 20; }
    combo1 = 5; combo2 = 10; combo3 = 15;

    combo_num = firstnum;

    // Check for state transitions
    if (start_flag == 1) {
        state = run;
    }
    // delay_by_nop(30000);

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
    }

    // Perform state tasks
    combo_num();

    // Check for state transitions
    if (win_flag == 1) {state = end_win;}
    else if (lose_flag == 1) {state = end_fail;}
    else if (SW2 == 0 ) {state = solved;}

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
    if (win_flag == 1) {state = end_win;}
    else if (lose_flag == 1) {state = end_fail;}

    // State Cleanup
    if (state != last_state) {
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
    }

    // Perform state tasks
    if (IFS0bits.T1IF == 1) {
        IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
        MODULE_LED_GREEN = !MODULE_LED_GREEN;           // toggle LED
    }

    // Check for state transitions

    // State Cleanup
    if (state != last_state) {
        MODULE_LED_GREEN = OFF;
                T1CONbits.TON = 0;      // turn off Timer1
    }
}

void end_fail(void) { // The master module said the game was lost
    // State Setup
    if (state != last_state) {
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

    // Check for state transitions

    // State Cleanup
    if (state != last_state) {
        // cleanup state here
        MODULE_LED_RED = OFF;
        T1CONbits.TON = 0;      // turn off Timer1
    }
}

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////


void ledoff(void) {
    LED1 = 0; delay_by_nop(1);
    LED2 = 0; delay_by_nop(1);
    LED3 = 0; delay_by_nop(1);
    // D0 = OFF;
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
