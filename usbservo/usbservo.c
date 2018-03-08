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
#include "usb.h"
#include <stdio.h>

#define TOGGLE_LED1         0
#define TOGGLE_LED2         1
#define TOGGLE_LED3         2
#define READ_SW1            3
#define READ_SW2            4
#define READ_SW3            5
#define READ_A0             6
#define SET_SERVO1          7
#define SET_SERVO2          8

#define SERVO_MIN_WIDTH     900e-6
#define SERVO_MAX_WIDTH     2.1e-3

uint16_t servo_offset, servo_multiplier;
WORD32 servo_temp;

void vendor_requests(void) {
    WORD temp;

    switch (USB_setup.bRequest) {
        case TOGGLE_LED1:
            LED1 = !LED1;
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case TOGGLE_LED2:
            LED2 = !LED2;
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case TOGGLE_LED3:
            LED3 = !LED3;
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case READ_SW1:
            BD[EP0IN].address[0] = SW1 ? 1 : 0;
            BD[EP0IN].bytecount = 1;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case READ_SW2:
            BD[EP0IN].address[0] = SW2 ? 1 : 0;
            BD[EP0IN].bytecount = 1;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case READ_SW3:
            BD[EP0IN].address[0] = SW3 ? 1 : 0;
            BD[EP0IN].bytecount = 1;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case READ_A0:
            temp.w = read_analog(A0_AN);
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];
            BD[EP0IN].bytecount = 2;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case SET_SERVO1:
            servo_temp.ul = (uint32_t)USB_setup.wValue.w * (uint32_t)servo_multiplier;
            OC1RS = servo_offset + servo_temp.w[1];
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case SET_SERVO2:
            servo_temp.ul = (uint32_t)USB_setup.wValue.w * (uint32_t)servo_multiplier;
            OC2RS = servo_offset + servo_temp.w[1];
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        default:
            USB_error_flags |= REQUEST_ERROR;
    }
}

int16_t main(void) {
    uint8_t *RPOR, *RPINR;
    WORD32 temp;

    init_elecanisms();

    servo_offset = (uint16_t)(FCY * SERVO_MIN_WIDTH);
    servo_multiplier = (uint16_t)(FCY * (SERVO_MAX_WIDTH - SERVO_MIN_WIDTH));

    // Configure pin D13 and D12 to produce hobby servo control signals
    // using the OC1 and OC2 modules, respectively.
    D13_DIR = OUT;      // configure D13 to be a digital output
    D13 = 0;            // set D13 low

    D12_DIR = OUT;      // configure D12 to be a digital output
    D12 = 0;            // set D12 low

    RPOR = (uint8_t *)&RPOR0;
    RPINR = (uint8_t *)&RPINR0;

    __builtin_write_OSCCONL(OSCCON & 0xBF);
    RPOR[D13_RP] = OC1_RP;  // connect the OC1 module output to pin D13
    RPOR[D12_RP] = OC2_RP;  // connect the OC2 module output to pin D12
    __builtin_write_OSCCONL(OSCCON | 0x40);

    OC1CON1 = 0x1C0F;   // configure OC1 module to use the peripheral
                        //   clock (i.e., FCY, OCTSEL<2:0> = 0b111),
                        //   TRIGSTAT = 1, and to operate in center-aligned 
                        //   PWM mode (OCM<2:0> = 0b111)
    OC1CON2 = 0x008B;   // configure OC1 module to trigger from Timer1
                        //   (OCTRIG = 1 and SYNCSEL<4:0> = 0b01011)

    // set OC1 pulse width to 1.5ms (i.e. halfway between 0.9ms and 2.1ms)
    servo_temp.ul = 0x8000 * (uint32_t)servo_multiplier;
    OC1RS = servo_offset + servo_temp.w[1];
    OC1R = 1;
    OC1TMR = 0;

    OC2CON1 = 0x1C0F;   // configure OC2 module to use the peripheral
                        //   clock (i.e., FCY, OCTSEL<2:0> = 0b111),
                        //   TRIGSTAT = 1, and to operate in center-aligned 
                        //   PWM mode (OCM<2:0> = 0b111)
    OC2CON2 = 0x008B;   // configure OC2 module to trigger from Timer1
                        //   (OCTRIG = 1 and SYNCSEL<4:0> = 0b01011)

    // set OC2 pulse width to 1.5ms (i.e. halfway between 0.9ms and 2.1ms)
    servo_temp.ul = 0x8000 * (uint32_t)servo_multiplier;
    OC2RS = servo_offset + servo_temp.w[1];
    OC2R = 1;
    OC2TMR = 0;

    T1CON = 0x0010;     // configure Timer1 to have a period of 20ms
    PR1 = 0x9C3F;

    TMR1 = 0;
    T1CONbits.TON = 1;

    USB_setup_vendor_callback = vendor_requests;
    init_usb();

    while (USB_USWSTAT != CONFIG_STATE) {
#ifndef USB_INTERRUPT
        usb_service();
#endif
    }
    while (1) {
#ifndef USB_INTERRUPT
        usb_service();
#endif
    }
}

