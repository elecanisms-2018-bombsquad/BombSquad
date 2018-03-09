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
#include "ajuart.h"

RINGBUFFER U1_tx_buffer, U1_rx_buffer;
uint8_t U1TXbuffer[U1_TX_BUFFER_LENGTH];
uint8_t U1RXbuffer[U1_RX_BUFFER_LENGTH];
uint16_t U1_tx_threshold;

void init_ajuart(void) {
    uint8_t *RPOR, *RPINR;

    // Configure audio jack UART pins and connect them to UART1
    AJ_TX_DIR = OUT; AJ_TX = 1;
    AJ_RX_DIR = IN;

    RPOR = (uint8_t *)&RPOR0;
    RPINR = (uint8_t *)&RPINR0;

    __builtin_write_OSCCONL(OSCCON & 0xBF);
    RPINR[U1RX_RP] = AJ_RX_RP;
    RPOR[AJ_TX_RP] = U1TX_RP;
    __builtin_write_OSCCONL(OSCCON | 0x40);

    U1MODE = 0x0008;                // configure UART1 for transmission at
    U1BRG = 0x0022;                 //   115,200 baud, no parity, 1 stop bit

    U1_tx_buffer.data = U1TXbuffer;
    U1_tx_buffer.length = U1_TX_BUFFER_LENGTH;
    U1_tx_buffer.head = 0;
    U1_tx_buffer.tail = 0;
    U1_tx_buffer.count = 0;
    U1_tx_threshold = 3 * U1_TX_BUFFER_LENGTH / 4;

    U1_rx_buffer.data = U1RXbuffer;
    U1_rx_buffer.length = U1_RX_BUFFER_LENGTH;
    U1_rx_buffer.head = 0;
    U1_rx_buffer.tail = 0;
    U1_rx_buffer.count = 0;

    U1STAbits.UTXISEL1 = 0;         // set UART1 UTXISEL<1:0> = 01, TX interrupt
    U1STAbits.UTXISEL0 = 1;         //   when all transmit operations are done

    IFS0bits.U1TXIF = 0;            // lower UART1 TX interrupt flag
    IEC0bits.U1TXIE = 1;            // enable UART1 TX interrupt

    IFS0bits.U1RXIF = 0;            // lower UART1 RX interrupt flag
    IEC0bits.U1RXIE = 1;            // enable UART1 RX interrupt

    U1MODEbits.UARTEN = 1;          // enable UART1 module
    U1STAbits.UTXEN = 1;            // enable UART1 data transmission
}

void __attribute__((interrupt, auto_psv)) _U1TXInterrupt(void) {
    uint8_t ch;

    IFS0bits.U1TXIF = 0;            // lower UART1 TX interrupt flag

    if (U1_tx_buffer.count == 0)    // if nothing left in UART1 TX buffer, 
        U1STAbits.UTXEN = 0;        //   disable data transmission

    while ((U1STAbits.UTXBF == 0) && (U1_tx_buffer.count != 0)) {
        disable_interrupts();
        ch = U1_tx_buffer.data[U1_tx_buffer.head];
        U1_tx_buffer.head++;
        if (U1_tx_buffer.head == U1_tx_buffer.length)
            U1_tx_buffer.head = 0;
        U1_tx_buffer.count--;
        enable_interrupts();
        U1TXREG = (uint16_t)ch;
    }
}

void __attribute__((interrupt, auto_psv)) _U1RXInterrupt(void) {
    IFS0bits.U1RXIF = 0;            // lower UART1 RX interrupt flag

    while ((U1STAbits.URXDA == 1) && (U1_rx_buffer.count != U1_rx_buffer.length)) {
        disable_interrupts();
        U1_rx_buffer.data[U1_rx_buffer.tail] = (uint8_t)U1RXREG;
        U1_rx_buffer.tail++;
        if (U1_rx_buffer.tail == U1_rx_buffer.length)
            U1_rx_buffer.tail = 0;
        U1_rx_buffer.count++;
        enable_interrupts();
    }
}

uint16_t U1_in_waiting(void) {
    return U1_rx_buffer.count;
}

void U1_flush_tx_buffer(void) {
    if (U1STAbits.UTXEN == 0)       // if UART1 transmission is disabled,
        U1STAbits.UTXEN = 1;        //   enable it
}

void U1_putc(uint8_t ch) {
    // Wait until UART1 TX buffer is not full
    while (U1_tx_buffer.count == U1_tx_buffer.length) {}

    disable_interrupts();
    U1_tx_buffer.data[U1_tx_buffer.tail] = ch;
    U1_tx_buffer.tail++;
    if (U1_tx_buffer.tail == U1_tx_buffer.length)
        U1_tx_buffer.tail = 0;
    U1_tx_buffer.count++;
    enable_interrupts();

    // If UART1 TX buffer is full enough, enable data transmission
    if (U1_tx_buffer.count >= U1_tx_threshold)
        U1STAbits.UTXEN = 1;
}

uint8_t U1_getc(void) {
    uint8_t ch;

    // Wait until UART1 RX buffer is not empty
    while (U1_rx_buffer.count == 0) {}

    disable_interrupts();
    ch = U1_rx_buffer.data[U1_rx_buffer.head];
    U1_rx_buffer.head++;
    if (U1_rx_buffer.head == U1_rx_buffer.length)
        U1_rx_buffer.head = 0;
    U1_rx_buffer.count--;
    enable_interrupts();

    return ch;
}

void U1_puts(uint8_t *str) {
    while (*str)
        U1_putc(*str++);
    U1_flush_tx_buffer();
}

void U1_gets(uint8_t *str, uint16_t len) {
    if (len == 0)
        return;

    if (len == 1) {
        *str = '\0';
        return;
    }

    while (1) {
        *str = U1_getc();
        if ((*str == '\r') || (len == 1))
            break;
        str++;
        len--;
    }
    *str = '\0';
}

void U1_gets_term(uint8_t *str, uint16_t len) {
    uint8_t *start;
    uint16_t left;

    if (len == 0)
        return;

    if (len == 1) {
        *str = '\0';
        return;
    }

    U1_putc(0x1B);                          // save current cursor position
    U1_putc('7');
    U1_flush_tx_buffer();
    start = str;
    left = len;
    while (1) {
        *str = U1_getc();                   // get a character
        if (*str == '\r')                   // if character is return,
            break;                          //   end the loop
        if (*str == 0x1B) {                 // if character is escape,
            U1_putc(0x1B);                  //   restore cursor position,
            U1_putc('8');
            U1_putc(0x1B);                  //   clear to end of line, and
            U1_putc('[');
            U1_putc('K');
            U1_flush_tx_buffer();
            str = start;                    //   start over at the beginning
            left = len;
            continue;
        }
        if ((*str == '\b') ||               // if character is backspace
            (*str == 0x7F)) {               //   or delete, 
            if (str > start) {              //   and we are not at the start, 
                U1_putc('\b');              //   erase the last character and
                U1_putc(' ');
                U1_putc('\b');
                U1_flush_tx_buffer();
                str--;                      //   back up the pointer,
                left++;
            } else {                        //   otherwise
                U1_putc('\a');              //   send alert/bell character
                U1_flush_tx_buffer();
            }
            continue;
        }
        if (left == 1) {                    // if string buffer is full,
            U1_putc('\a');                  //   send alert/bell character
            U1_flush_tx_buffer();
            continue;
        }
        if ((*str >= 32) && (*str < 127)) { // if character is printable,
            U1_putc(*str);                  //   echo the received character
            U1_flush_tx_buffer();
            str++;                          //   and advance the pointer
            left--;
        }
    }
    *str = '\0';                            // terminarte the string with null
    U1_putc('\n');                          // send newline and
    U1_putc('\r');                          //   carriage return
    U1_flush_tx_buffer();
}

int16_t write(int16_t handle, void *buffer, uint16_t len) {
    int16_t i;

    switch (handle) {
        case 0:
        case 1:
        case 2:
            for (i = 0; i < len; i++)
                U1_putc(*(uint8_t *)buffer++);
            U1_flush_tx_buffer();
            break;
        default:
            break;
    }
    return len;
}

