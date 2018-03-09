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

#define GREEN       0
#define BLUE        1
#define RED         2

#define GREEN_LED   LED2
#define RED_LED     LED1
#define BLUE_LED    LED3

uint16_t state, counter;

int16_t main(void) {
    init_elecanisms();

    T1CON = 0x0030;         // set Timer1 period to 0.5s
    PR1 = 0x7A11;

    TMR1 = 0;               // set Timer1 count to 0
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag

    state = GREEN;    
    GREEN_LED = ON;

    while (1) {
        switch (state) {
            case GREEN:
                if (SW1 == 0) {
                    state = BLUE;
                    GREEN_LED = OFF;
                    BLUE_LED = ON;
                    IFS0bits.T1IF = 0;
                    TMR1 = 0;
                    T1CONbits.TON = 1;
                    counter = 0;
                }
                break;
            case BLUE:
                if (IFS0bits.T1IF == 1) {
                    IFS0bits.T1IF = 0;
                    BLUE_LED = !BLUE_LED;
                    counter++;
                }
                if (SW2 == 0) {
                    state = GREEN;
                    BLUE_LED = OFF;
                    GREEN_LED = ON;
                    T1CONbits.TON = 0;
                } else if (counter == 20) {
                    state = RED;
                    BLUE_LED = OFF;
                    RED_LED = ON;
                    T1CONbits.TON = 0;
                }
                break;
            case RED:
                break;
        }
    }
}

