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
#ifndef _I2C_H_
#define _I2C_H_

#include <stdint.h>

void init_i2c(void);

typedef struct {
    uint16_t *I2CxRCV;
    uint16_t *I2CxTRN;
    uint16_t *I2CxBRG;
    uint16_t *I2CxCON;
    uint16_t *I2CxSTAT;
    uint16_t *I2CxADD;
    uint16_t *I2CxMSK;
    uint16_t *IFSy;
    uint8_t MI2CxIF;
} _I2C;

extern _I2C i2c1, i2c2, i2c3;

void i2c_init(_I2C *self, uint16_t *I2CxRCV, uint16_t *I2CxTRN,
              uint16_t *I2CxBRG, uint16_t *I2CxCON,
              uint16_t *I2CxSTAT, uint16_t *I2CxADD,
              uint16_t *I2CxMSK, uint16_t *IFSy,
              uint8_t MI2CxIF);
void i2c_open(_I2C *self, float freq);
void i2c_close(_I2C *self);
void i2c_start(_I2C *self);
void i2c_restart(_I2C *self);
void i2c_ack(_I2C *self);
void i2c_nak(_I2C *self);
void i2c_stop(_I2C *self);
void i2c_idle(_I2C *self);
uint8_t i2c_putc(_I2C *self, uint8_t ch);
uint8_t i2c_getc(_I2C *self);
void blocking_delay_us(uint16_t N);


#endif
