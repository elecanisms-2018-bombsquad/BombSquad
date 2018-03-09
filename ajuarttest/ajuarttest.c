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
#include "ajuart.h"
#include "usb.h"
#include <stdio.h>

#define TOGGLE_LED1         0
#define TOGGLE_LED2         1
#define TOGGLE_LED3         2
#define READ_SW1            3
#define READ_SW2            4
#define READ_SW3            5
#define UART_PUTS           6
#define UART_GETS           7
#define UART_IN_WAITING     8
#define HELLO_WORLD         9

#define BUFFER_LENGTH       1024

uint8_t data_buffer[BUFFER_LENGTH];

void send_data_buffer(void) {
    uint16_t i;

    for (i = 0; i < USB_request.setup.wLength.w; i++)
        U1_putc(data_buffer[i]);
    U1_flush_tx_buffer();
}

void vendor_requests(void) {
    WORD temp;
    uint16_t i;

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
        case UART_PUTS:
            if (USB_setup.wLength.w > BUFFER_LENGTH)
                USB_error_flags |= REQUEST_ERROR;
            else {
                USB_request.setup.bmRequestType = USB_setup.bmRequestType;
                USB_request.setup.bRequest = USB_setup.bRequest;
                USB_request.setup.wValue.w = USB_setup.wValue.w;
                USB_request.setup.wIndex.w = USB_setup.wIndex.w;
                USB_request.setup.wLength.w = USB_setup.wLength.w;
                USB_request.bytes_left.w = USB_setup.wLength.w;
                USB_request.data_ptr = data_buffer;
                USB_request.done_callback = send_data_buffer;
                USB_out_callbacks[0] = usb_receive_data_packet;
            }
            break;
        case UART_GETS:
            if (USB_setup.wLength.w > BUFFER_LENGTH)
                USB_error_flags |= REQUEST_ERROR;
            else {
                for (i = 0; i < USB_setup.wLength.w; i++)
                    data_buffer[i] = U1_getc();
                USB_request.setup.bmRequestType = USB_setup.bmRequestType;
                USB_request.setup.bRequest = USB_setup.bRequest;
                USB_request.setup.wValue.w = USB_setup.wValue.w;
                USB_request.setup.wIndex.w = USB_setup.wIndex.w;
                USB_request.setup.wLength.w = USB_setup.wLength.w;
                USB_request.bytes_left.w = USB_setup.wLength.w;
                USB_request.data_ptr = data_buffer;
                USB_in_callbacks[0] = usb_send_data_packet;
                usb_send_data_packet();
            }
            break;
        case UART_IN_WAITING:
            temp.w = U1_in_waiting();
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];
            BD[EP0IN].bytecount = 2;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case HELLO_WORLD:
            printf("Hello world!\n");
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        default:
            USB_error_flags |= REQUEST_ERROR;
    }
}

int16_t main(void) {
    init_elecanisms();
    init_ajuart();

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

