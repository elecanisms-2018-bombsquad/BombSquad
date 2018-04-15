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

#ifndef _ELECANISMS_H_
#define _ELECANISMS_H_

#include "pic24fj.h"
#include "common.h"
#include <stdint.h>

#define PROGMEM __attribute__((space(prog)))

// LED pin definitions
#define LED1                LATDbits.LATD7
#define LED2                LATFbits.LATF0
#define LED3                LATFbits.LATF1

#define LED1_DIR            TRISDbits.TRISD7
#define LED2_DIR            TRISFbits.TRISF0
#define LED3_DIR            TRISFbits.TRISF1

// Tactile switch pin definitions
#define SW1                 PORTCbits.RC15
#define SW2                 PORTCbits.RC12
#define SW3                 PORTBbits.RB12

#define SW1_DIR             TRISCbits.TRISC15
#define SW2_DIR             TRISCbits.TRISC12
#define SW3_DIR             TRISBbits.TRISB12

// Digital header pin definitions
#define D0                  PORTDbits.RD5
#define D1                  PORTDbits.RD4
#define D2                  PORTFbits.RF4
#define D3                  PORTFbits.RF5
#define D4                  PORTBbits.RB15
#define D5                  PORTFbits.RF3
#define D6                  PORTDbits.RD8
#define D7                  PORTDbits.RD11
#define D8                  PORTDbits.RD9
#define D9                  PORTDbits.RD10
#define D10                 PORTDbits.RD0
#define D11                 PORTDbits.RD1
#define D12                 PORTDbits.RD2
#define D13                 PORTDbits.RD3

#define D0_DIR              TRISDbits.TRISD5
#define D1_DIR              TRISDbits.TRISD4
#define D2_DIR              TRISFbits.TRISF4
#define D3_DIR              TRISFbits.TRISF5
#define D4_DIR              TRISBbits.TRISB15
#define D5_DIR              TRISFbits.TRISF3
#define D6_DIR              TRISDbits.TRISD8
#define D7_DIR              TRISDbits.TRISD11
#define D8_DIR              TRISDbits.TRISD9
#define D9_DIR              TRISDbits.TRISD10
#define D10_DIR             TRISDbits.TRISD0
#define D11_DIR             TRISDbits.TRISD1
#define D12_DIR             TRISDbits.TRISD2
#define D13_DIR             TRISDbits.TRISD3

#define D0_PUE              CNPU1bits.CN14PUE
#define D1_PUE              CNPU1bits.CN13PUE
#define D2_PUE              CNPU2bits.CN17PUE
#define D3_PUE              CNPU2bits.CN18PUE
#define D4_PUE              CNPU1bits.CN12PUE
#define D5_PUE              CNPU5bits.CN71PUE
#define D6_PUE              CNPU4bits.CN53PUE
#define D7_PUE              CNPU4bits.CN56PUE
#define D8_PUE              CNPU4bits.CN54PUE
#define D9_PUE              CNPU4bits.CN55PUE
#define D10_PUE             CNPU4bits.CN49PUE
#define D11_PUE             CNPU3bits.CN46PUE
#define D12_PUE             CNPU4bits.CN51PUE
#define D13_PUE             CNPU4bits.CN52PUE

#define D0_PDE              CNPD1bits.CN14PDE
#define D1_PDE              CNPD1bits.CN13PDE
#define D2_PDE              CNPD2bits.CN17PDE
#define D3_PDE              CNPD2bits.CN18PDE
#define D4_PDE              CNPD1bits.CN12PDE
#define D5_PDE              CNPD5bits.CN71PDE
#define D6_PDE              CNPD4bits.CN53PDE
#define D7_PDE              CNPD4bits.CN56PDE
#define D8_PDE              CNPD4bits.CN54PDE
#define D9_PDE              CNPD4bits.CN55PDE
#define D10_PDE             CNPD4bits.CN49PDE
#define D11_PDE             CNPD3bits.CN46PDE
#define D12_PDE             CNPD4bits.CN51PDE
#define D13_PDE             CNPD4bits.CN52PDE

#define D0_CNEN              CNEN1bits.CN14IE
#define D1_CNEN              CNEN1bits.CN13IE
#define D2_CNEN              CNEN2bits.CN17IE
#define D3_CNEN              CNEN2bits.CN18IE
#define D4_CNEN              CNEN1bits.CN12IE
#define D5_CNEN              CNEN5bits.CN71IE
#define D6_CNEN              CNEN4bits.CN53IE
#define D7_CNEN              CNEN4bits.CN56IE
#define D8_CNEN              CNEN4bits.CN54IE
#define D9_CNEN              CNEN4bits.CN55IE
#define D10_CNEN             CNEN4bits.CN49IE
#define D11_CNEN             CNEN3bits.CN46IE
#define D12_CNEN             CNEN4bits.CN51IE
#define D13_CNEN             CNEN4bits.CN52IE

#define D0_RP               20
#define D1_RP               25
#define D2_RP               10
#define D3_RP               17
#define D4_RP               29
#define D5_RP               16
#define D6_RP               2
#define D7_RP               12
#define D8_RP               4
#define D9_RP               3
#define D10_RP              11
#define D11_RP              24
#define D12_RP              23
#define D13_RP              22

// Analog header pin definitions
#define A0                  PORTBbits.RB5
#define A1                  PORTBbits.RB4
#define A2                  PORTBbits.RB3
#define A3                  PORTBbits.RB2
#define A4                  PORTBbits.RB1
#define A5                  PORTBbits.RB0

#define A0_DIR              TRISBbits.TRISB5
#define A1_DIR              TRISBbits.TRISB4
#define A2_DIR              TRISBbits.TRISB3
#define A3_DIR              TRISBbits.TRISB2
#define A4_DIR              TRISBbits.TRISB1
#define A5_DIR              TRISBbits.TRISB0

#define A0_RP               18
#define A1_RP               28
#define A3_RP               13
#define A4_RP               1
#define A5_RP               0

#define A0_AN               5
#define A1_AN               4
#define A2_AN               3
#define A3_AN               2
#define A4_AN               1
#define A5_AN               0


#define INT1_RP             1
#define INT2_RP             2
#define INT3_RP             3
#define INT4_RP             4

#define MOSI1_RP            7
#define SCK1OUT_RP          8
#define MOSI2_RP            10
#define SCK2OUT_RP          11
#define MOSI3_RP            32
#define SCK3OUT_RP          33

#define MISO1_RP            40
#define SCK1IN_RP           41
#define MISO2_RP            44
#define SCK2IN_RP           45
#define MISO3_RP            56
#define SCK3IN_RP           57

#define OC1_RP              18
#define OC2_RP              19
#define OC3_RP              20
#define OC4_RP              21
#define OC5_RP              22
#define OC6_RP              23
#define OC7_RP              24
#define OC8_RP              25
#define OC9_RP              35

#define U1TX_RP             3
#define U1RTS_RP            4
#define U2TX_RP             5
#define U2RTS_RP            6
#define U3TX_RP             28
#define U3RTS_RP            29
#define U4TX_RP             30
#define U4RTS_RP            31

#define U1RX_RP             36
#define U1CTS_RP            37
#define U2RX_RP             38
#define U2CTS_RP            39
#define U3RX_RP             35
#define U3CTS_RP            43
#define U4RX_RP             54
#define U4CTS_RP            55


#define FALSE               0
#define TRUE                1

#define OFF                 0
#define ON                  1

#define OUT                 0
#define IN                  1

void init_elecanisms(void);
uint16_t read_analog(uint16_t pin_an);

#endif
