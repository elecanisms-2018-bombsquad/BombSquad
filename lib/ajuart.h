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

#ifndef _AJUART_H_
#define _AJUART_H_

#include "elecanisms.h"

// Audio jack pin UART pin definitions
#define AJ_TX               PORTGbits.RG6
#define AJ_RX               PORTGbits.RG7

#define AJ_TX_DIR           TRISGbits.TRISG6
#define AJ_RX_DIR           TRISGbits.TRISG7

#define AJ_TX_RP            21
#define AJ_RX_RP            26

#define U1_TX_BUFFER_LENGTH 1024
#define U1_RX_BUFFER_LENGTH 1024

typedef struct {
    uint8_t *data;
    uint16_t length;
    uint16_t head;
    uint16_t tail;
    uint16_t count;
} RINGBUFFER;

extern RINGBUFFER U1_tx_buffer, U1_rx_buffer;
extern uint8_t U1TXbuffer[];
extern uint8_t U1RXbuffer[];
extern uint16_t U1_tx_threshold;

void init_ajuart(void);

uint16_t U1_in_waiting(void);
void U1_flush_tx_buffer(void);
void U1_putc(uint8_t ch);
uint8_t U1_getc(void);
void U1_puts(uint8_t *str);
void U1_gets(uint8_t *str, uint16_t len);
void U1_gets_term(uint8_t *str, uint16_t len);

#endif

