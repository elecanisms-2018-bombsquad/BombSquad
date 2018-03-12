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
#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>

void init_timer(void);

typedef struct _TIMER {
    uint16_t *TxCON;
    uint16_t *PRx;
    uint16_t *TMRx;
    uint16_t *IFSy;
    uint16_t *IECy;
    uint8_t flagbit;
    uint16_t octselnum;
    uint16_t ocsyncselnum;
    uint16_t aftercount;
    void (*every)(struct _TIMER *self);
    void (*after)(struct _TIMER *self);
} _TIMER;

extern _TIMER timer1, timer2, timer3, timer4, timer5;
extern _TIMER *timerDelay;

void timer_init(_TIMER *self, uint16_t *TxCON, uint16_t *PRx,
                uint16_t *TMRx, uint16_t *IFSy, uint16_t *IECy,
                uint8_t flagbit, uint16_t octselnum, uint16_t ocsyncselnum);
void timer_setPeriod(_TIMER *self, float period);
float timer_period(_TIMER *self);
void timer_setFreq(_TIMER *self, float freq);
float timer_freq(_TIMER *self);
float timer_time(_TIMER *self);
void timer_start(_TIMER *self);
void timer_stop(_TIMER *self);
uint16_t timer_flag(_TIMER *self);
void timer_lower(_TIMER *self);
uint16_t timer_read(_TIMER *self);
void timer_enableInterrupt(_TIMER *self);
void timer_disableInterrupt(_TIMER *self);
void timer_every(_TIMER *self, float interval, void (*callback)(_TIMER *self));
void timer_after(_TIMER *self, float delay, uint16_t num_times,
                 void (*callback)(_TIMER *self));
void timer_delayMicro(uint16_t uS);
void timer_cancel(_TIMER *self);

void timer_initDelayMicro(_TIMER *timer);
void timer_delayMicro(uint16_t usec);

#endif
