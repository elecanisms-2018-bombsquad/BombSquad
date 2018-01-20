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

void init_elecanisms(void) {
    CLKDIV = 0x0100;        // RCDIV = 001 (4MHz, div2), 
                            // CPDIV = 00 (FOSC = 32MHz, FCY = 16MHz)

    // Configure all pins on the digital headers to be inputs
    D0_DIR = IN;
    D1_DIR = IN;
    D2_DIR = IN;
    D3_DIR = IN;
    D4_DIR = IN;
    D5_DIR = IN;
    D6_DIR = IN;
    D7_DIR = IN;
    D8_DIR = IN;
    D9_DIR = IN;
    D10_DIR = IN;
    D11_DIR = IN;
    D12_DIR = IN;
    D13_DIR = IN;

    // Configure all pins on the analog header to be inputs
    A0_DIR = IN;
    A1_DIR = IN;
    A2_DIR = IN;
    A3_DIR = IN;
    A4_DIR = IN;
    A5_DIR = IN;

    // Make all pins digital I/Os except for A0,...,A5
    ANSB = 0x003F;
    ANSC = 0;
    ANSD = 0;
    ANSF = 0;
    ANSG = 0;

    // Configure and enable the ADC module: 
    //   set output data format to integer (FORM<1:0> = 0b00)
    //   set conversion trigger source select bits to internal counter 
    //     (SSRC<2:0> = 0b111)
    //   set auto-sample time bits to 12*TAD (SAMC<4:0> = 12)
    //   set conversion clock select bits to 64*TCY (ADCS<7:0> = 64)
    //   set initial channel selection to A0
    AD1CON1 = 0x00E0; 
    AD1CON2 = 0;
    AD1CON3 = 0x0C40;
    AD1CHS = A0_AN;
    AD1CON1bits.ADON = 1;

    // Configure LED pins as outputs, set to low (off)
    LED1_DIR = OUT; LED1 = 0;
    LED2_DIR = OUT; LED2 = 0;
    LED3_DIR = OUT; LED3 = 0;

    // Configure SW pins as inputs
    SW1_DIR = IN;
    SW2_DIR = IN;
    SW3_DIR = IN;
}

uint16_t read_analog(uint16_t pin_an) {
    AD1CHS = pin_an;
    AD1CON1bits.SAMP = 1;
    while (AD1CON1bits.DONE == 0) {}
    return ADC1BUF0;
}

