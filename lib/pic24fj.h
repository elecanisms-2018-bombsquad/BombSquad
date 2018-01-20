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

#ifndef _PIC24FJ_H_
#define _PIC24FJ_H_

#ifdef __PIC24FJ64GB002__
#include <p24FJ64GB002.h>

#define HAS_TIMER1
#define HAS_TIMER2
#define HAS_TIMER3
#define HAS_TIMER4
#define HAS_TIMER5

#define HAS_IC1
#define HAS_IC2
#define HAS_IC3
#define HAS_IC4
#define HAS_IC5

#define HAS_OC1
#define HAS_OC2
#define HAS_OC3
#define HAS_OC4
#define HAS_OC5

#define HAS_UART1
#define HAS_UART2

#define HAS_SPI1
#define HAS_SPI2

#define HAS_I2C1
#define HAS_I2C2

#endif

#ifdef __PIC24FJ128GB206__
#include <p24FJ128GB206.h>

#define HAS_TIMER1
#define HAS_TIMER2
#define HAS_TIMER3
#define HAS_TIMER4
#define HAS_TIMER5

#define HAS_IC1
#define HAS_IC2
#define HAS_IC3
#define HAS_IC4
#define HAS_IC5
#define HAS_IC6
#define HAS_IC7
#define HAS_IC8
#define HAS_IC9

#define HAS_OC1
#define HAS_OC2
#define HAS_OC3
#define HAS_OC4
#define HAS_OC5
#define HAS_OC6
#define HAS_OC7
#define HAS_OC8
#define HAS_OC9

#define HAS_UART1
#define HAS_UART2
#define HAS_UART3
#define HAS_UART4

#define HAS_SPI1
#define HAS_SPI2
#define HAS_SPI3

#define HAS_I2C1
#define HAS_I2C2
#define HAS_I2C3

#endif

#ifdef __PIC24FJ256GB206__
#include <p24FJ256GB206.h>

#define HAS_TIMER1
#define HAS_TIMER2
#define HAS_TIMER3
#define HAS_TIMER4
#define HAS_TIMER5

#define HAS_IC1
#define HAS_IC2
#define HAS_IC3
#define HAS_IC4
#define HAS_IC5
#define HAS_IC6
#define HAS_IC7
#define HAS_IC8
#define HAS_IC9

#define HAS_OC1
#define HAS_OC2
#define HAS_OC3
#define HAS_OC4
#define HAS_OC5
#define HAS_OC6
#define HAS_OC7
#define HAS_OC8
#define HAS_OC9

#define HAS_UART1
#define HAS_UART2
#define HAS_UART3
#define HAS_UART4

#define HAS_SPI1
#define HAS_SPI2
#define HAS_SPI3

#define HAS_I2C1
#define HAS_I2C2
#define HAS_I2C3

#endif

#ifdef __PIC24FJ128GC006__
#include <p24FJ128GC006.h>

#define HAS_TIMER1
#define HAS_TIMER2
#define HAS_TIMER3
#define HAS_TIMER4
#define HAS_TIMER5

#define HAS_IC1
#define HAS_IC2
#define HAS_IC3
#define HAS_IC4
#define HAS_IC5
#define HAS_IC6
#define HAS_IC7
#define HAS_IC8
#define HAS_IC9

#define HAS_OC1
#define HAS_OC2
#define HAS_OC3
#define HAS_OC4
#define HAS_OC5
#define HAS_OC6
#define HAS_OC7
#define HAS_OC8
#define HAS_OC9

#define HAS_UART1
#define HAS_UART2
#define HAS_UART3
#define HAS_UART4

#define HAS_SPI1
#define HAS_SPI2

#define HAS_I2C1
#define HAS_I2C2

#endif

#endif
