/*
** Copyright (c) 2018, Bradley A. Minch
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**     2. Redistributions in binary form must reproduce the above copyright
**        notice, this list of conditions and the following disclaimer in the
**        documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
*/
#include "elecanisms.h"

volatile uint16_t current_led = 0;

void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    switch (current_led) {  // Toggle whichever LED is the current one
      case 1 :
        LED1 = !LED1;
        LED2 = 0;
        break;
      default :
        LED2 = !LED2;
        LED1 = 0;
        break;
    }
}

void __attribute__((interrupt, auto_psv)) _T2Interrupt(void) {
    IFS0bits.T2IF = 0;      // lower Timer2 interrupt flag
    T2CONbits.TON = 0;      // turn off timer
    if (PORTDbits.RD10 == 1) {          // Sample D9 after debounce
      current_led ^= 1;
    }
}

void __attribute__((interrupt, auto_psv)) _INT3Interrupt(void) {
    IFS3bits.INT3IF = 0;      // lower INT3 interrupt flag
    TMR2 = 0;                 // reset debounce Timer2
    IFS0bits.T2IF = 0;        // lower Timer2 Interrupt flag
    T2CONbits.TON = 1;        // start Timer2
}

int16_t main(void) {
    init_elecanisms();

    LED2 = ON;
    current_led = 0;

// Timer 1 Setup
    T1CON = 0x0030;         // set Timer1 period to 0.25s, prescaler 256 match 15624
    PR1 = 0x3D08;

    TMR1 = 0;               // set Timer1 count to 0
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    IEC0bits.T1IE = 1;      // enable Timer1 interrupt
    T1CONbits.TON = 1;      // turn on Timer1

// Timer 2 Setup
    T2CON = 0x0020;         // set Timer2 period to 10 ms for debounce
    PR2 = 0x2710;           // prescaler 16, match value 10000

    TMR2 = 0;               // set Timer2 to 0
    IFS0bits.T2IF = 0;      // lower T2 interrupt flag
    IEC0bits.T2IE = 1;      // enable T2 interrupt
    T2CONbits.TON = 0;      // make sure T2 isn't on

// INT3 Setup
    RPINR1bits.INT3R = INT3_RP; // Configure interrupt 3 on RP3, pin D9 on board

    IFS3bits.INT3IF = 0;    // lower interrupt flag for INT3
    IEC3bits.INT3IE = 1;    // enable INT3 interrupt


    while (1) {
      LED3 = current_led;   // Debug output on LED3
    }
}
