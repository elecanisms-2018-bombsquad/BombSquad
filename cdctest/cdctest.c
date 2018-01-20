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
#include "cdc.h"
#include <stdio.h>

void set_config_callback(void) {
    USB_setup_class_callback = cdc_setup_callback;

    BD[EP1IN].bytecount = 0;
    BD[EP1IN].address = EP1_IN_buffer;
    BD[EP1IN].status = UOWN | DTS | DTSEN;
    U1EP1 = ENDPT_IN_ONLY;

    BD[EP2OUT].bytecount = 64;
    BD[EP2OUT].address = EP2_OUT_buffer;
    BD[EP2OUT].status = UOWN | DTSEN;
    USB_out_callbacks[2] = cdc_rx_service;

    BD[EP2IN].bytecount = 0;
    BD[EP2IN].address = EP2_IN_buffer;
    BD[EP2IN].status = UOWN | DTS | DTSEN;
    U1EP2 = ENDPT_NON_CONTROL;
    USB_in_callbacks[2] = cdc_tx_service;
}

int16_t main(void) {
    uint8_t buffer[80];

    init_elecanisms();

    init_cdc();
    USB_set_config_callback = set_config_callback;
    init_usb();

    while (USB_USWSTAT != CONFIG_STATE) {
#ifndef USB_INTERRUPT
        usb_service();
#endif
    }

    printf("Hello world!!\n\r");

    while (1) {
#ifndef USB_INTERRUPT
        usb_service();
#endif
        cdc_puts(">>> ");
        cdc_gets_term(buffer, 80);
        cdc_putc('"');
        cdc_puts(buffer);
        cdc_putc('"');
        cdc_putc('\n');
        cdc_putc('\r');
    }
}

