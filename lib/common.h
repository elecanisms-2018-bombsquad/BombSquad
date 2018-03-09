/*
** Copyright (c) 2013, Bradley A. Minch
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

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>

#define init_clock()    CLKDIV = 0x0100     // RCDIV = 001 (4MHz, div2), 
                                            // CPDIV = 00 (FOSC = 32MHz, FCY = 16MHz)

#define FCY     16e6
#define TCY     62.5e-9

#ifndef NULL
#define NULL 0
#endif

#define peek(addr)              *(addr)
#define poke(addr, val)         *(addr) = val
#define bitread(addr, bit)      (((*(addr))&(1<<bit)) ? 1:0)
#define bitset(addr, bit)       *(addr) |= 1<<bit
#define bitclear(addr, bit)     *(addr) &= ~(1<<bit)
#define bitflip(addr, bit)      *(addr) ^= 1<<bit

#define disable_interrupts()    __asm__ volatile("disi #0x3FFF")
#define enable_interrupts()     DISICNT = 0

typedef union {
    int16_t i;
    uint16_t w;
    uint8_t b[2];
} WORD;

typedef union {
    int32_t l;
    uint32_t ul;
    uint16_t w[2];
    uint8_t b[4];
} WORD32;

uint8_t parity(uint16_t v);

#endif
