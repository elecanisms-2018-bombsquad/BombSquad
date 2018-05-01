/*
 * Code word module for BombSquad project
 * Alexander Hoppe
 * Elecanisms SP 2018
 */


#include "elecanisms.h"
#include "lcd.h"
#include "i2c_reg.h"
#include "codeword.h"
#include "peripheral_core.h"
#include "i2c_address_space.h"
#include "ajuart.h"

#define LETTERS_PER_COLUMN  6
#define MODULE_LED_RED      D10
#define MODULE_LED_GREEN    D11

char* dispptr;
char *codeSet;
char *set0, *set1, *set2, *set3, *set4;
char* codeword;
uint8_t i0, i1, i2, i3, i4;
uint16_t rand_val;

typedef void (*STATE_HANDLER_T)(void);

// forward declaration of module modes
void setup(void);
void run(void);
void solved(void);
void end_win(void);
void end_fail(void);

STATE_HANDLER_T state, last_state;
uint16_t counter;

_LCD lcd1;

void main(void) {
    init_elecanisms();
    init_ajuart();
    // Setup rocker pins as inputs and set pull-up resistors
    toggleSwitchSetup();

    i2c_init(1e3);
    lcd_init(&lcd1, 0x06, 'A'); //Setup LCD screen (type A i/o extender)
    lcd_clear(&lcd1);  // Clears _LCD objects from previous array

    i2c2_init(157);                      // Initializes I2C on I2C2
    I2C2ADD = MODULE_CODEWORD_ADDR>>1;   // Set the device address (7-bit register)
    I2C2MSK = 0;                         // Set mask to 0 (only this address matters)
    _SI2C2IE = 1;                        // Enable i2c slave interrupt

    rand_val = read_analog(A0); // Set up the seed

    // Add more random noise
    uint8_t i, j;
    for (i=0; i<20; i++) {
        for (j=0; j<read_analog(A0); j++) {
            rand_next();
            delay_by_nop(read_analog(A0));
        }
    }

    U1_putc((uint8_t)rand_val >> 8);
    U1_putc((uint8_t)rand_val);
    U1_putc('\r');
    U1_putc('\n');
    U1_flush_tx_buffer();

    // set codeword and set of letters
    char _codeSet[36];
    for(i=0; i < 36; i++) {
        _codeSet[i] = letterSets[rand_val%15][i];
    }
    codeSet = _codeSet;

    char tmp[7] = " ";
    for (i=0; i<5; i++){
        tmp[i] = codeSet[i];
    }
    codeword = tmp;

    // setup column sets of letters
    char _set0[7] = " ";
    for (i=0; i<6; i++){
        _set0[i] = codeSet[i+5];
    }
    set0 = _set0;

    char _set1[7] = " ";
    for (i=0; i<6; i++){
        _set1[i] = codeSet[i+11];
    }
    set1 = _set1;

    char _set2[7] = " ";
    for (i=0; i<6; i++){
        _set2[i] = codeSet[i+17];
    }
    set2 = _set2;

    char _set3[7] = " ";
    for (i=0; i<6; i++){
        _set3[i] = codeSet[i+23];
    }
    set3 = _set3;

    char _set4[7] = " ";
    for (i=0; i<6; i++){
        _set4[i] = codeSet[i+29];
    }
    set4 = _set4;

    // initialize indices of each column
    i0 = rand_val%5;
    rand_next();
    i1 = rand_val%5;
    rand_next();
    i2 = rand_val%5;
    rand_next();
    i3 = rand_val%5;
    rand_next();
    i4 = rand_val%5;

    char dispstring[17] = " o  o  o  o  o ";
    dispptr = dispstring;
    lcd_print2(&lcd1, dispptr, "");

    // Timer 2 Setup
    T2CON = 0x0020;         // set Timer2 period to 10 ms for debounce
    PR2 = 0x2710;           // prescaler 16, match value 10000

    TMR2 = 0;               // set Timer2 to 0
    IFS0bits.T2IF = 0;      // lower T2 interrupt flag
    IEC0bits.T2IE = 1;      // enable T2 interrupt
    T2CONbits.TON = 0;      // make sure T2 isn't on

    state = setup;

    while (1) {
        state();
    }
}

// STATE MACHINE FUNCTIONS /////////////////////////////////////////////////////

void setup(void) { // Waits for master module to start the game
    // State Setup
    if (state != last_state) {
        last_state = state;
        MODULE_LED_GREEN = ON; delay_by_nop(1);
        MODULE_LED_RED = ON;
        complete_flag = 0;
        // setup state here
    }

    // Perform state tasks

    // Check for state transitions
    // if ((start_flag == 1) || (SW2 == 0)){
    //     state = run;
    // }

    if (start_flag == 1) {
        state = run;
    }

    // State Cleanup
    if (state != last_state) {
        // cleanup state here
        MODULE_LED_GREEN = OFF; delay_by_nop(1);
        MODULE_LED_RED = ON;
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
    updateDisplay();
    if (SW1 == 0) { //TODO: Add real submit button pin mapping
        if ((set0[i0] == codeword[0]) &&
            (set1[i1] == codeword[1]) &&
            (set2[i2] == codeword[2]) &&
            (set3[i3] == codeword[3]) &&
            (set4[i4] == codeword[4])) {
                state = solved;
        } else {
            num_strikes++;
            LED3 = OFF;
        }
    }
    lcd_print2(&lcd1, dispptr, "");
    delay_by_nop(30000);

    // Check for state transitions
    if (win_flag == 1) {
        state = end_win;
    } else if (lose_flag == 1) {
        state = end_fail;
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
        // setup state here
        last_state = state;
        LED3 = ON;
        complete_flag = 1;
        MODULE_LED_GREEN = ON;
    }

    // Perform state tasks
    lcd_print2(&lcd1, dispptr, "  --CORRECT--  ");
    delay_by_nop(30000);

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
    }

    // Perform state tasks
    if (IFS0bits.T1IF == 1) {
        IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
        MODULE_LED_GREEN = !MODULE_LED_GREEN;           // toggle LED
    }

    // Check for state transitions

    // State Cleanup
    if (state != last_state) {
        // cleanup state here
        MODULE_LED_GREEN = OFF;
        T1CONbits.TON = 0;      // turn off Timer1
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

    // Check for state transitions

    // State Cleanup
    if (state != last_state) {
        // cleanup state here
        MODULE_LED_RED = OFF;
        T1CONbits.TON = 0;      // turn off Timer1
    }
}

// ISRs ////////////////////////////////////////////////////////////////////////

void __attribute__((interrupt, auto_psv)) _CNInterrupt(void) {
    IFS1bits.CNIF = 0;      // lower INT3 interrupt flag
    TMR2 = 0;                 // reset debounce Timer2
    IFS0bits.T2IF = 0;        // lower Timer2 Interrupt flag
    T2CONbits.TON = 1;        // start Timer2
    LED1 = ON;
}

void __attribute__((interrupt, auto_psv)) _T2Interrupt(void) {
    IFS0bits.T2IF = 0;      // lower Timer2 interrupt flag
    T2CONbits.TON = 0;      // turn off timer
    LED1 = OFF;

    // Sample pins and increment/decrement index of character displayed
    if(!D0) {i0 = (i0+1)% LETTERS_PER_COLUMN;} else
    if(!D1) {i0 = (i0 - 1);
        if(i0 > 5) {i0 = LETTERS_PER_COLUMN - 1;}
    }

    if(!D12) {i1 = (i1+1)% LETTERS_PER_COLUMN;} else
    if(!D13) {i1 = (i1 - 1);}
    if(i1 > 5) {i1 += LETTERS_PER_COLUMN;}

    if(!D4) {i2 = (i2+1)% LETTERS_PER_COLUMN;} else
    if(!D5) {i2 = (i2 - 1);}
    if(i2 > 5) {i2 += LETTERS_PER_COLUMN;}

    if(!D6) {i3 = (i3+1)% LETTERS_PER_COLUMN;} else
    if(!D7) {i3 = (i3 - 1);}
    if(i3 > 5) {i3 += LETTERS_PER_COLUMN;}

    if(!D8) {i4 = (i4+1)% LETTERS_PER_COLUMN;} else
    if(!D9) {i4 = (i4 - 1);}
    if(i4 > 5) {i4 += LETTERS_PER_COLUMN;}

    updateDisplay();
}


// HELPER FUNCTIONS ////////////////////////////////////////////////////////////

void updateDisplay(void) {
    dispptr[1] = set0[i0];
    dispptr[4] = set1[i1];
    dispptr[7] = set2[i2];
    dispptr[10] = set3[i3];
    dispptr[13] = set4[i4];
}

void toggleSwitchSetup(void) {
    // Initialize pins as inputs
    D0_DIR = 1; //D0 as input
    D1_DIR = 1; //D1 as input

    D4_DIR = 1; //D4 as input
    D5_DIR = 1; //D5 as input
    D6_DIR = 1; //D6 as input
    D7_DIR = 1; //D7 as input
    D8_DIR = 1; //D8 as input
    D9_DIR = 1; //D9 as input
    D10_DIR = 0; // D10 and D11 are module LEDs
    D11_DIR = 0;
    D12_DIR = 1;//D12 as input
    D13_DIR = 1;//D13 as input

    D0_PUE = 1; //D0 pullup enable
    D1_PUE = 1; //D1 pullup enable

    D4_PUE = 1; //D4 pullup enable
    D5_PUE = 1; //D5 pullup enable
    D6_PUE = 1; //D6 pullup enable
    D7_PUE = 1; //D7 pullup enable
    D8_PUE = 1; //D8 pullup enable
    D9_PUE = 1; //D9 pullup enable
    D12_PUE = 1;//D12 pullup enable
    D13_PUE = 1;//D13 pullup enable

    D0_CNEN = 1; //D0 interrupt enable
    D1_CNEN = 1; //D1 interrupt enable

    D4_CNEN = 1; //D4 interrupt enable
    D5_CNEN = 1; //D5 interrupt enable
    D6_CNEN = 1; //D6 interrupt enable
    D7_CNEN = 1; //D7 interrupt enable
    D8_CNEN = 1; //D8 interrupt enable
    D9_CNEN = 1; //D9 interrupt enable
    D12_CNEN = 1;//D12 interrupt enable
    D13_CNEN = 1;//D13 interrupt enable

    IFS1bits.CNIF = 0; // lower CN interrupt flag
    IEC1bits.CNIE = 1; // Enable CN interrupt module
}

void rand_next(void) {
    uint16_t val;

    // See "A List of Maximum Period NLFSRs" by Elena Dubrova, p. 7
    val = (rand_val ^ (rand_val >> 2) ^ (rand_val >> 13) ^ ((rand_val >> 2) & (rand_val >> 3))) & 1;
    rand_val = (rand_val >> 1) | (val << 15);
}
