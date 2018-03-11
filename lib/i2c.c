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
#include <p24FJ128GB206.h>
#include "common.h"
#include "i2c.h"

_I2C i2c1, i2c2, i2c3;
_PIN SCL3, SDA3;

void init_i2c(void) {
    // init_pin();

    // pin_init(&SDA3, (uint16_t *)&PORTE, (uint16_t *)&TRISE,
    //          (uint16_t *)NULL, 7, -1, 0, -1, (uint16_t *)NULL);
    // pin_init(&SCL3, (uint16_t *)&PORTE, (uint16_t *)&TRISE,
    //          (uint16_t *)NULL, 6, -1, 0, -1, (uint16_t *)NULL);

    i2c_init(&i2c1, (uint16_t *)&I2C1RCV, (uint16_t *)&I2C1TRN,
             (uint16_t *)&I2C1BRG, (uint16_t *)&I2C1CON,
             (uint16_t *)&I2C1STAT, (uint16_t *)&I2C1ADD,
             (uint16_t *)&I2C1MSK, (uint16_t *)&IFS1, 1,
             &D[8], &D[9]);
    i2c_init(&i2c2, (uint16_t *)&I2C2RCV, (uint16_t *)&I2C2TRN,
             (uint16_t *)&I2C2BRG, (uint16_t *)&I2C2CON,
             (uint16_t *)&I2C2STAT, (uint16_t *)&I2C2ADD,
             (uint16_t *)&I2C2MSK, (uint16_t *)&IFS3, 2,
             &D[2], &D[3]);
    i2c_init(&i2c3, (uint16_t *)&I2C3RCV, (uint16_t *)&I2C3TRN,
             (uint16_t *)&I2C3BRG, (uint16_t *)&I2C3CON,
             (uint16_t *)&I2C3STAT, (uint16_t *)&I2C3ADD,
             (uint16_t *)&I2C3MSK, (uint16_t *)&IFS5, 5);
}

void i2c_init(_I2C *self, uint16_t *I2CxRCV, uint16_t *I2CxTRN,
              uint16_t *I2CxBRG, uint16_t *I2CxCON,
              uint16_t *I2CxSTAT, uint16_t *I2CxADD,
              uint16_t *I2CxMSK, uint16_t *IFSy,
              uint8_t MI2CxIF) {
    self->I2CxRCV = I2CxRCV;
    self->I2CxTRN = I2CxTRN;
    self->I2CxBRG = I2CxBRG;
    self->I2CxCON = I2CxCON;
    self->I2CxSTAT = I2CxSTAT;
    self->I2CxADD = I2CxADD;
    self->I2CxMSK = I2CxMSK;
    self->IFSy = IFSy;
    self->MI2CxIF = MI2CxIF;
}

void i2c_open(_I2C *self, float freq) {
    uint16_t temp;

    // Disable I2C module
    *(self->I2CxCON) = 0;
    *(self->I2CxSTAT) = 0;

    // remove last year's pin-in-use checking code

    // if ((self->SDA->owner==NULL) && (self->SCL->owner==NULL)) {
    //     // Both SDA and SCL are available, so assign to the I2C module
    //     pin_digitalIn(self->SDA);
    //     pin_digitalIn(self->SCL);
    //     self->SDA->owner = (void *)self;
    //     self->SDA->write = NULL;
    //     self->SDA->read = NULL;
    //     self->SCL->owner = (void *)self;
    //     self->SCL->write = NULL;
    //     self->SCL->read = NULL;
    // } else {
    //     return; // At least one of SDA and SCL are being used by another
    //             // peripheral
    // }
    // Clip freq to be in allowable range of values
    if (freq<(FCY/(256.+(FCY/10e6))))
        freq = FCY/(256.+(FCY/10e6));
    if (freq>(FCY/(4.+(FCY/10e6))))
        freq = FCY/(4.+(FCY/10e6));
    // Compute BRG value to obtain closest value to that specified
    *(self->I2CxBRG) = (uint16_t)((FCY/freq)-(FCY/10e6)+0.5)-1;
    bitset(self->I2CxCON, 15);  // Set I2Cx enable bit
}

void i2c_close(_I2C *self) {
    *(self->I2CxCON) = 0;
    *(self->I2CxSTAT) = 0;

    // remove last year's pin check code

    // if (self->SDA->owner==(void *)self) {
    //     pin_digitalIn(self->SDA);
    //     self->SDA->owner = NULL;
    // }
    // if (self->SCL->owner==(void *)self) {
    //     pin_digitalIn(self->SCL);
    //     self->SCL->owner = NULL;
    // }
}

void i2c_start(_I2C *self) {
    bitset(self->I2CxCON, 0);   // Initiate a Start condition on I2Cx
    while (bitread(self->I2CxCON, 0)==1) {}     // Wait until Start condition
                                                //   clears
}

void i2c_restart(_I2C *self) {
    bitset(self->I2CxCON, 1);   // Initiate a Repeated Start condition on I2Cx
    while (bitread(self->I2CxCON, 1)==1) {}     // Wait until Repeated Start
                                                //   condition clears
}

void i2c_ack(_I2C *self) {
    bitclear(self->I2CxCON, 5); // Send ACK during Acknowledge
    bitset(self->I2CxCON, 4);   // Initiate an Acknowledge condition on I2Cx
    while (bitread(self->I2CxCON, 4)==1) {}     // Wait until Acknowledge
                                                //   condition clears
}

void i2c_nak(_I2C *self) {
    bitset(self->I2CxCON, 5);   // Send NACK during Acknowledge
    bitset(self->I2CxCON, 4);   // Initiate an Acknowldege condition on I2Cx
    while (bitread(self->I2CxCON, 4)==1) {}     // Wait until Acknowledge
                                                //   condition clears
}

void i2c_stop(_I2C *self) {
    bitset(self->I2CxCON, 2);   // Initiate a Stop condition on I2Cx
    while (bitread(self->I2CxCON, 2)==1) {}     // Wait until Stop condition
                                                //   clears
}

void i2c_idle(_I2C *self) {
    while (((*(self->I2CxCON))&0x1F) ||         // Wait until Start, Restart,
           (bitread(self->I2CxSTAT, 14)==1)) {} //   Stop, Receive, Acknowledge,
                                                //   and Transmit conditions
                                                //   are all clear on I2Cx
}

uint8_t i2c_putc(_I2C *self, uint8_t ch) {
    bitclear(self->IFSy, self->MI2CxIF);    // Lower master I2Cx interrupt flag
    *(self->I2CxTRN) = (uint16_t)ch;        // Initiate I2Cx data transmission
    while (bitread(self->I2CxSTAT, 0)==1) {}    // Wait until tranmit completes
                                                //   (8 clock cycles on SCLx)
    while (bitread(self->IFSy, self->MI2CxIF)==0) {}    // Wait for 9th clock
                                                        //   cycle on SCLx
    bitclear(self->IFSy, self->MI2CxIF);    // Lower master I2Cx interrupt flag
    return bitread(self->I2CxSTAT, 15);     // Return ACKSTAT bit of I2CxSTAT
}

uint8_t i2c_getc(_I2C *self) {
    bitset(self->I2CxCON, 3);       // Initiate a Receive on I2Cx
    while (bitread(self->I2CxCON, 3)==1) {}     // Wait until Receive condition
                                                //   clears on I2Cx
    bitclear(self->I2CxSTAT, 6);                // Clear I2COV bit
    return (uint8_t)(*(self->I2CxRCV));         // Return the received byte
}

void blocking_delay_us(uint16_t N) {
  uint16_t j;
  while(N > 0) {
    for(j=0;j < 16000; j++);
    __asm__("nop");
  }
}
