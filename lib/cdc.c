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

#include "pic24fj.h"
#include "cdc.h"

uint8_t EP1_IN_buffer[10];
uint8_t EP2_OUT_buffer[MAX_PACKET_SIZE];
uint8_t EP2_IN_buffer[MAX_PACKET_SIZE];

struct CDC_line_coding_struct {
    WORD32 dwDTERate;
    uint8_t bCharFormat;
    uint8_t bParityType;
    uint8_t bDataBits;
} CDC_line_coding;
uint16_t CDC_control_signal_bitmap;

struct CDC_ring_buffer {
    uint8_t *data;
    uint16_t length;
    uint16_t head;
    uint16_t tail;
    uint16_t count;
} CDC_TX_buffer, CDC_RX_buffer;

uint8_t TXbuf[TX_BUFFER_SIZE], RXbuf[RX_BUFFER_SIZE];

void cdc_set_line_coding_out_callback(void) {
    CDC_line_coding.dwDTERate.b[0] = BD[EP0OUT].address[0];
    CDC_line_coding.dwDTERate.b[1] = BD[EP0OUT].address[1];
    CDC_line_coding.dwDTERate.b[2] = BD[EP0OUT].address[2];
    CDC_line_coding.dwDTERate.b[3] = BD[EP0OUT].address[3];
    CDC_line_coding.bCharFormat = BD[EP0OUT].address[4];
    CDC_line_coding.bParityType = BD[EP0OUT].address[5];
    CDC_line_coding.bDataBits = BD[EP0OUT].address[6];
    USB_out_callbacks[0] = (USB_CALLBACK_T)NULL;
}

void cdc_setup_callback(void) {
    switch (USB_setup.bRequest) {
        case SEND_ENCAPSULATED_COMMAND:
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;  // send packet as DATA1, set UOWN bit
            break;
        case GET_ENCAPSULATED_RESPONSE:
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;  // send packet as DATA1, set UOWN bit
            break;
        case SET_LINE_CODING:
            USB_request.setup.bmRequestType = USB_setup.bmRequestType;  // save setup packet to continue processing 
            USB_request.setup.bRequest = USB_setup.bRequest;            // a SET_LINE_CODING request when the OUT 
            USB_request.setup.wValue.w = USB_setup.wValue.w;            // request arrives
            USB_request.setup.wIndex.w = USB_setup.wIndex.w;
            USB_request.setup.wLength.w = USB_setup.wLength.w;
            USB_out_callbacks[0] = cdc_set_line_coding_out_callback;
            break;
        case GET_LINE_CODING:
            BD[EP0IN].address[0] = CDC_line_coding.dwDTERate.b[0];
            BD[EP0IN].address[1] = CDC_line_coding.dwDTERate.b[1];
            BD[EP0IN].address[2] = CDC_line_coding.dwDTERate.b[2];
            BD[EP0IN].address[3] = CDC_line_coding.dwDTERate.b[3];
            BD[EP0IN].address[4] = CDC_line_coding.bCharFormat;
            BD[EP0IN].address[5] = CDC_line_coding.bParityType;
            BD[EP0IN].address[6] = CDC_line_coding.bDataBits;
            BD[EP0IN].bytecount = 7;
            BD[EP0IN].status = UOWN | DTS | DTSEN;  // send packet as DATA1, set UOWN bit
            break;
        case SET_CONTROL_LINE_STATE:
            CDC_control_signal_bitmap = USB_setup.wValue.w;
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;  // send packet as DATA1, set UOWN bit
            break;
        case SET_COMM_FEATURE:
        case GET_COMM_FEATURE:
        case CLEAR_COMM_FEATURE:
        case SEND_BREAK:
        default:
            USB_error_flags |= REQUEST_ERROR;
    }
}

void init_cdc(void) {
    CDC_line_coding.dwDTERate.ul = 115200;  // set initial line coding paramters to 115,200 baud
    CDC_line_coding.bCharFormat = 0;        // ...1 stop bit,
    CDC_line_coding.bParityType = 0;        // ...no parity,
    CDC_line_coding.bDataBits = 8;          // ...and 8 data bits

    CDC_control_signal_bitmap = 0;

    CDC_TX_buffer.data = TXbuf;
    CDC_TX_buffer.length = TX_BUFFER_SIZE;
    CDC_TX_buffer.head = 0;
    CDC_TX_buffer.tail = 0;
    CDC_TX_buffer.count = 0;

    CDC_RX_buffer.data = RXbuf;
    CDC_RX_buffer.length = RX_BUFFER_SIZE;
    CDC_RX_buffer.head = 0;
    CDC_RX_buffer.tail = 0;
    CDC_RX_buffer.count = 0;
}

void cdc_tx_service(void) {
    uint8_t packet_length, i;

    if (!(BD[EP2IN].status & UOWN)) {   // see if UOWN bit of EP2 IN status register is clear (i.e., PIC owns EP2 IN buffer)
        if (CDC_TX_buffer.count < MAX_PACKET_SIZE) 
            packet_length = CDC_TX_buffer.count;
        else
            packet_length = MAX_PACKET_SIZE;
        for (i = 0; i < packet_length; i++) {
            BD[EP2IN].address[i] = CDC_TX_buffer.data[CDC_TX_buffer.head];
            CDC_TX_buffer.head++;
            if (CDC_TX_buffer.head == CDC_TX_buffer.length)
                CDC_TX_buffer.head = 0;
        }
        CDC_TX_buffer.count -= packet_length;
        BD[EP2IN].bytecount = packet_length;
        BD[EP2IN].status = ((BD[EP2IN].status ^ DTS) & DTS) | UOWN | DTSEN; // toggle DATA01 bit, clear the PIDs bits, and set the UOWN and DTS bits
    }
}

void cdc_rx_service(void) {
    uint8_t i;

    if (!(BD[EP2OUT].status & UOWN)) {  // see if UOWN bit of EP2 OUT status register is clear (i.e., PIC owns EP2 OUT buffer)
        if ((BD[EP2OUT].bytecount + CDC_RX_buffer.count) <= CDC_RX_buffer.length) {
            for (i = 0; i < BD[EP2OUT].bytecount; i++) {
                CDC_RX_buffer.data[CDC_RX_buffer.tail] = BD[EP2OUT].address[i];
                CDC_RX_buffer.tail++;
                if (CDC_RX_buffer.tail == CDC_RX_buffer.length)
                    CDC_RX_buffer.tail = 0;
            }
            CDC_RX_buffer.count += BD[EP2OUT].bytecount;
            BD[EP2OUT].bytecount = 64;
            BD[EP2OUT].status = ((BD[EP2OUT].status ^ DTS) & DTS) | UOWN | DTSEN;   // toggle DATA01 bit, clear the PIDs bits, and set the UOWN and DTS bits
        } else
            USB_error_flags |= REQUEST_ERROR;
    }
}

void cdc_putc(uint8_t ch) {
    while (CDC_TX_buffer.count == CDC_TX_buffer.length) {
#ifndef USB_INTERRUPT
        usb_service();
#endif
    }
    disable_interrupts();
    CDC_TX_buffer.data[CDC_TX_buffer.tail] = ch;
    CDC_TX_buffer.tail++;
    if (CDC_TX_buffer.tail == CDC_TX_buffer.length)
        CDC_TX_buffer.tail = 0;
    CDC_TX_buffer.count++;
    enable_interrupts();
}

uint8_t cdc_getc(void) {
    uint8_t ch;

    while (CDC_RX_buffer.count == 0) {
#ifndef USB_INTERRUPT
        usb_service();
#endif
    }
    disable_interrupts();
    ch = CDC_RX_buffer.data[CDC_RX_buffer.head];
    CDC_RX_buffer.head++;
    if (CDC_RX_buffer.head == CDC_RX_buffer.length)
        CDC_RX_buffer.head = 0;
    CDC_RX_buffer.count--;
    enable_interrupts();
    return ch;
}

void cdc_puts(uint8_t *str) {
    while (*str)
        cdc_putc(*str++);
}

void cdc_gets(uint8_t *str, uint16_t len) {
    if (len == 0)
        return;

    if (len == 1) {
        *str = '\0';
        return;
    }

    while (1) {
        *str = cdc_getc();
        if ((*str == '\r') || (len == 1))
            break;
        str++;
        len--;
    }
    *str = '\0';
}

void cdc_gets_term(uint8_t *str, uint16_t len) {
    uint8_t *start;
    uint16_t left;

    if (len == 0)
        return;

    if (len == 1) {
        *str = '\0';
        return;
    }

    cdc_putc(0x1B);                        // Save current cursor position
    cdc_putc('7');
    start = str;
    left = len;
    while (1) {
        *str = cdc_getc();                 // Get a character
        if (*str == '\r')                  // If character is return,
            break;                         //   end the loop.
        if (*str == 0x1B) {                // If character is escape,
            cdc_putc(0x1B);                //   restore cursor position,
            cdc_putc('8');
            cdc_putc(0x1B);                //   clear to end of line, and
            cdc_putc('[');
            cdc_putc('K');
            str = start;                   //   start over at the beginning.
            left = len;
            continue;
        }
        if ((*str == '\b') ||              // If character is backspace
            (*str == 0x7F)) {              //   or delete, 
            if (str > start) {             //   and we are not at the start, 
                cdc_putc('\b');            //   erase the last character and
                cdc_putc(' ');
                cdc_putc('\b');
                str--;                     //   back up the pointer,
                left++;
            } else                         //   otherwise
                cdc_putc('\a');            //   send alert/bell character.
            continue;
        }
        if (left == 1) {                   // If string buffer is full,
            cdc_putc('\a');                //   send alert/bell character
            continue;
        }
        if ((*str >= 32) && (*str < 127)) {// If character is printable,
            cdc_putc(*str);                //   echo the received character
            str++;                         //   and advance the pointer.
            left--;
        }
    }
    *str = '\0';                           // Terminarte the string with null
    cdc_putc('\n');                        // Send newline and
    cdc_putc('\r');                        //   carriage return
}

int16_t write(int16_t handle, void *buffer, uint16_t len) {
    int16_t i;

    switch (handle) {
        case 0:
        case 1:
        case 2:
            for (i = 0; i < len; i++)
                cdc_putc(*(uint8_t *)buffer++);
            break;
        default:
            break;
    }
    return len;
}

