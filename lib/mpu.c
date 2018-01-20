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

#include "mpu.h"

#ifdef MPU_MAG
uint8_t mpu_mag_cal_vals[3];
uint16_t mpu_mag_delay_count;
#endif

void init_mpu(void) {
    uint8_t *RPOR, *RPINR;

    // Configure MPU-9250 pins and connect them to SPI1
    MPU_CSN_DIR = OUT; MPU_CSN = 1;
    MPU_SCK_DIR = OUT; MPU_SCK = 0;
    MPU_MOSI_DIR = OUT; MPU_MOSI = 0;
    MPU_MISO_DIR = IN;
    MPU_INT_DIR = IN;

    RPOR = (uint8_t *)&RPOR0;
    RPINR = (uint8_t *)&RPINR0;

    __builtin_write_OSCCONL(OSCCON & 0xBF);
    RPINR[MISO1_RP] = MPU_MISO_RP;
    RPOR[MPU_MOSI_RP] = MOSI1_RP;
    RPOR[MPU_SCK_RP] = SCK1OUT_RP;
    __builtin_write_OSCCONL(OSCCON | 0x40);

    SPI1CON1 = 0x0132;              // SPI mode = 0, SCK freq = 1 MHz
    SPI1CON2 = 0;
    SPI1STAT = 0x8000;

    // Reset the MPU-9250
    mpu_writeReg(MPU_PWR_MGMT_1, 0x80);
    // Use DLPF, set gyro bandwidth to 184 Hz and temp bandwidth to 188 Hz
    mpu_writeReg(MPU_CONFIG, 0x01);
    // Set gyro range to +/-2000 dps
    mpu_writeReg(MPU_GYRO_CONFIG, 0x18);
    // Set accel range to +/-4 g
    mpu_writeReg(MPU_ACCEL_CONFIG, 0x08);
    // Set accel data rate, enable accel LPF, set bandwith to 184 Hz
    mpu_writeReg(MPU_ACCEL_CONFIG2, 0x09);
    // Configure INT pin to latch and clear on any read
    mpu_writeReg(MPU_INT_PIN_CFG, 0x30);

#ifdef MPU_MAG
    // Set I2C master mode, reset I2C slave module, and put serial interface
    // in SPI mode
    mpu_writeReg(MPU_USER_CTRL, 0x30);
    // Confiugre I2C slave interface for a 400-kHz clock
    mpu_writeReg(MPU_I2C_MST_CTRL, 0x0D);
    // Set I2C Slave 0 address to AK8963's I2C address
    mpu_writeReg(MPU_I2C_SLV0_ADDR, MAG_I2C_ADDR);

    // Reset the AK8963
    mpu_writeReg(MPU_I2C_SLV0_REG, MAG_CNTL2);
    mpu_writeReg(MPU_I2C_SLV0_DO, 0x01);
    mpu_writeReg(MPU_I2C_SLV0_CTRL, 0x81);
    for (mpu_mag_delay_count = MPU_MAG_DELAY; 
         mpu_mag_delay_count > 0; 
         mpu_mag_delay_count--) {}

    // Put AK8963 in fuse ROM access mode
    mpu_writeReg(MPU_I2C_SLV0_REG, MAG_CNTL1);
    mpu_writeReg(MPU_I2C_SLV0_DO, 0x0F);
    mpu_writeReg(MPU_I2C_SLV0_CTRL, 0x81);
    for (mpu_mag_delay_count = MPU_MAG_DELAY; 
         mpu_mag_delay_count > 0; 
         mpu_mag_delay_count--) {}

    // Read ASA values from AK8963
    mpu_writeReg(MPU_I2C_SLV0_ADDR, MAG_I2C_ADDR | 0x80);
    mpu_writeReg(MPU_I2C_SLV0_REG, MAG_ASAX);
    mpu_writeReg(MPU_I2C_SLV0_CTRL, 0x83);
    for (mpu_mag_delay_count = MPU_MAG_DELAY; 
         mpu_mag_delay_count > 0; 
         mpu_mag_delay_count--) {}
    mpu_readRegs(MPU_EXT_SENS_DATA_00, mpu_mag_cal_vals, 3);

    // Put AK8963 into power-down mode
    mpu_writeReg(MPU_I2C_SLV0_ADDR, MAG_I2C_ADDR);
    mpu_writeReg(MPU_I2C_SLV0_REG, MAG_CNTL1);
    mpu_writeReg(MPU_I2C_SLV0_DO, 0x00);
    mpu_writeReg(MPU_I2C_SLV0_CTRL, 0x81);
    for (mpu_mag_delay_count = MPU_MAG_DELAY; 
         mpu_mag_delay_count > 0; 
         mpu_mag_delay_count--) {}

    // Configure AK8963 for continuous 16-bit measurement mode at 100 Hz
    mpu_writeReg(MPU_I2C_SLV0_REG, MAG_CNTL1);
    mpu_writeReg(MPU_I2C_SLV0_DO, 0x16);
    mpu_writeReg(MPU_I2C_SLV0_CTRL, 0x81);
    for (mpu_mag_delay_count = MPU_MAG_DELAY; 
         mpu_mag_delay_count > 0; 
         mpu_mag_delay_count--) {}
#endif

    mpu_set_accel_scale(0);
    mpu_set_gyro_scale(0);
}

void mpu_writeReg(uint8_t address, uint8_t value) {
    uint16_t ret;

    if (address <= 0x7E) {
        MPU_CSN = 0;

        SPI1BUF = (uint16_t)address;
        while (SPI1STATbits.SPIRBF == 0) {}
        ret = SPI1BUF;

        SPI1BUF = (uint16_t)value;
        while (SPI1STATbits.SPIRBF == 0) {}
        ret = SPI1BUF;

        MPU_CSN = 1;
    }
}

uint8_t mpu_readReg(uint8_t address) {
    uint16_t ret;

    if (address <= 0x7E) {
        MPU_CSN = 0;

        SPI1BUF = (uint16_t)(0x80 | address);
        while (SPI1STATbits.SPIRBF == 0) {}
        ret = SPI1BUF;

        SPI1BUF = 0;
        while (SPI1STATbits.SPIRBF == 0) {}
        ret = SPI1BUF;

        MPU_CSN = 1;

        return (uint8_t)ret;
    } else
        return 0xFF;
}

void mpu_writeRegs(uint8_t address, uint8_t *buffer, uint8_t n) {
    uint16_t ret;
    uint8_t i;

    if (address + n <= 0x7E) {
        MPU_CSN = 0;

        SPI1BUF = (uint16_t)address;
        while (SPI1STATbits.SPIRBF == 0) {}
        ret = SPI1BUF;

        for (i = 0; i < n; i++) {
            SPI1BUF = (uint16_t)buffer[i];
            while (SPI1STATbits.SPIRBF == 0) {}
            ret = SPI1BUF;
        }

        MPU_CSN = 1;
    }
}

void mpu_readRegs(uint8_t address, uint8_t *buffer, uint8_t n) {
    uint16_t ret;
    uint8_t i;

    if (address + n <= 0x7E) {
        MPU_CSN = 0;

        SPI1BUF = (uint16_t)(0x80 | address);
        while (SPI1STATbits.SPIRBF == 0) {}
        ret = SPI1BUF;

        for (i = 0; i < n; i++) {
            SPI1BUF = 0;
            while (SPI1STATbits.SPIRBF == 0) {}
            buffer[i] = (uint8_t)SPI1BUF;
        }

        MPU_CSN = 1;
    } else {
        for (i = 0; i < n; i++)
            buffer[i] = 0xFF;
    }
}

void mpu_set_accel_scale(uint16_t scale) {
    if (scale < 4)
        mpu_writeReg(MPU_ACCEL_CONFIG, (uint8_t)(scale << 3));
}

uint16_t mpu_get_accel_scale(void) {
    return (uint16_t)(mpu_readReg(MPU_ACCEL_CONFIG) >> 3);
}

void mpu_set_gyro_scale(uint16_t scale) {
    if (scale < 4)
        mpu_writeReg(MPU_GYRO_CONFIG, (uint8_t)(scale << 3));
}

uint16_t mpu_get_gyro_scale(void) {
    return (uint16_t)(mpu_readReg(MPU_GYRO_CONFIG) >> 3);
}

uint16_t mpu_whoami(void) {
    return (uint16_t)mpu_readReg(MPU_WHO_AM_I);
}

void mpu_read_accel(struct mpu_sensor_vals *values) {
    uint8_t buffer[6];
    WORD temp;

    mpu_readRegs(MPU_ACCEL_XOUT_H, buffer, 6);

    temp.b[0] = buffer[1];
    temp.b[1] = buffer[0];
    values->x = temp.i;

    temp.b[0] = buffer[3];
    temp.b[1] = buffer[2];
    values->y = temp.i;

    temp.b[0] = buffer[5];
    temp.b[1] = buffer[4];
    values->z = temp.i;
}

void mpu_read_gyro(struct mpu_sensor_vals *values) {
    uint8_t buffer[6];
    WORD temp;

    mpu_readRegs(MPU_GYRO_XOUT_H, buffer, 6);

    temp.b[0] = buffer[1];
    temp.b[1] = buffer[0];
    values->x = temp.i;

    temp.b[0] = buffer[3];
    temp.b[1] = buffer[2];
    values->y = temp.i;

    temp.b[0] = buffer[5];
    temp.b[1] = buffer[4];
    values->z = temp.i;
}

int16_t mpu_read_temp(void) {
    uint8_t buffer[2];
    WORD temp;

    mpu_readRegs(MPU_TEMP_OUT_H, buffer, 2);

    temp.b[0] = buffer[1];
    temp.b[1] = buffer[0];
    return temp.i;
}

#ifdef MPU_MAG
uint16_t mpu_mag_whoami(void) {
    mpu_writeReg(MPU_I2C_SLV0_ADDR, MAG_I2C_ADDR | 0x80);
    mpu_writeReg(MPU_I2C_SLV0_REG, MAG_WIA);
    mpu_writeReg(MPU_I2C_SLV0_CTRL, 0x81);
    for (mpu_mag_delay_count = MPU_MAG_DELAY; 
         mpu_mag_delay_count > 0; 
         mpu_mag_delay_count--) {}

    return (uint16_t)mpu_readReg(MPU_EXT_SENS_DATA_00);
}

void mpu_read_mag(struct mpu_sensor_vals *values) {
    uint8_t buffer[6];
    WORD temp;

    mpu_writeReg(MPU_I2C_SLV0_ADDR, MAG_I2C_ADDR | 0x80);
    mpu_writeReg(MPU_I2C_SLV0_REG, MAG_HXL);
    mpu_writeReg(MPU_I2C_SLV0_CTRL, 0x87);
    for (mpu_mag_delay_count = MPU_MAG_DELAY; 
         mpu_mag_delay_count > 0; 
         mpu_mag_delay_count--) {}

    mpu_readRegs(MPU_EXT_SENS_DATA_00, buffer, 6);

    temp.b[0] = buffer[0];
    temp.b[1] = buffer[1];
    values->x = temp.i;

    temp.b[0] = buffer[2];
    temp.b[1] = buffer[3];
    values->y = temp.i;

    temp.b[0] = buffer[4];
    temp.b[1] = buffer[5];
    values->z = temp.i;
}
#endif

