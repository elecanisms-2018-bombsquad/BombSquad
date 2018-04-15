/*
 * Code word module for BombSquad project
 * Alexander Hoppe
 * Elecanisms SP 2018
 */


#include "elecanisms.h"
#include "lcd.h"
#include "i2c_reg.h"

char* dispptr;

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

    // Sample pins
    if(!D0) {dispptr[5] = '^';} else
    if(!D1) {dispptr[5] = 'v';}
    else {dispptr[5] = 'o';}
    if(!D2) {dispptr[6] = '^';} else
    if(!D3) {dispptr[6] = 'v';}
    else {dispptr[6] = 'o';}
    if(!D4) {dispptr[7] = 'v';} else
    if(!D5) {dispptr[7] = '^';}
    else {dispptr[7] = 'o';}
    if(!D6) {dispptr[8] = 'v';} else
    if(!D7) {dispptr[8] = '^';}
    else {dispptr[8] = 'o';}
    if(!D8) {dispptr[9] = 'v';} else
    if(!D9) {dispptr[9] = '^';}
    else {dispptr[9] = 'o';}
}

void toggleSwitchSetup(void) {
    // Initialize pins as inputs
    D0_DIR = 1; //D0 as input
    D1_DIR = 1; //D1 as input
    D2_DIR = 1; //D2 as input
    D3_DIR = 1; //D3 as input
    D4_DIR = 1; //D4 as input
    D5_DIR = 1; //D5 as input
    D6_DIR = 1; //D6 as input
    D7_DIR = 1; //D7 as input
    D8_DIR = 1; //D8 as input
    D9_DIR = 1; //D9 as input

    D0_PUE = 1; //D0 pullup enable
    D1_PUE = 1; //D1 pullup enable
    D2_PUE = 1; //D2 pullup enable
    D3_PUE = 1; //D3 pullup enable
    D4_PUE = 1; //D4 pullup enable
    D5_PUE = 1; //D5 pullup enable
    D6_PUE = 1; //D6 pullup enable
    D7_PUE = 1; //D7 pullup enable
    D8_PUE = 1; //D8 pullup enable
    D9_PUE = 1; //D9 pullup enable

    D0_CNEN = 1; //D0 interrupt enable
    D1_CNEN = 1; //D1 interrupt enable
    D2_CNEN = 1; //D2 interrupt enable
    D3_CNEN = 1; //D3 interrupt enable
    D4_CNEN = 1; //D4 interrupt enable
    D5_CNEN = 1; //D5 interrupt enable
    D6_CNEN = 1; //D6 interrupt enable
    D7_CNEN = 1; //D7 interrupt enable
    D8_CNEN = 1; //D8 interrupt enable
    D9_CNEN = 1; //D9 interrupt enable

    IFS1bits.CNIF = 0; // lower CN interrupt flag
    IEC1bits.CNIE = 1; // Enable CN interrupt module
}

void main(void) {
    init_elecanisms();
    // Setup rocker pins as inputs and set pull-up resistors
    toggleSwitchSetup();
    // Initializes I2C on I2C3
    i2c_init(1e3);
    // Initializes LCD structs with addresses
    _LCD lcd1;
    lcd_init(&lcd1, 0x06, 'A');
    lcd_clear(&lcd1);  // Clears _LCD objects from previous array
    lcd_print2(&lcd1,"Code word","Module");
    delay_by_nop(30000);


    char dispstring[17] = "     ooooo     ";
    dispptr = dispstring;
    lcd_print2(&lcd1, dispptr, "");

    // Timer 2 Setup
    T2CON = 0x0020;         // set Timer2 period to 1 ms for debounce
    PR2 = 0x3e8;           // prescaler 16, match value 1000

    TMR2 = 0;               // set Timer2 to 0
    IFS0bits.T2IF = 0;      // lower T2 interrupt flag
    IEC0bits.T2IE = 1;      // enable T2 interrupt
    T2CONbits.TON = 0;      // make sure T2 isn't on



    while (1) {
        lcd_print2(&lcd1, dispptr, "");
        delay_by_nop(3000);
    }

}
