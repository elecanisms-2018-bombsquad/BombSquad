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
#include "mpu.h"
#include "usb.h"

#define TOGGLE_LED1         0
#define TOGGLE_LED2         1
#define TOGGLE_LED3         2
#define READ_SW1            3
#define READ_SW2            4
#define READ_SW3            5
#define MPU_WRITE_REG       6
#define MPU_READ_REG        7
#define MPU_WRITE_REGS      8
#define MPU_READ_REGS       9
#define MPU_SET_ACCEL_SCALE 10
#define MPU_GET_ACCEL_SCALE 11
#define MPU_SET_GYRO_SCALE  12
#define MPU_GET_GYRO_SCALE  13
#define MPU_WHOAMI          14
#define MPU_READ_ACCEL      15
#define MPU_READ_GYRO       16
#define MPU_READ_TEMP       17

#ifdef MPU_MAG
#define MPU_MAG_WHOAMI      18
#define MPU_READ_MAG        19
#define MPU_READ_MAG_CAL    20
#endif

#define BUFFER_LENGTH       64

uint8_t receive_buffer[BUFFER_LENGTH];

void write_regs_from_buffer(void) {
    mpu_writeRegs(USB_request.setup.wValue.b[0], receive_buffer, USB_request.setup.wLength.b[0]);
}

void vendor_requests(void) {
    struct mpu_sensor_vals values;
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
        case MPU_WRITE_REG:
            mpu_writeReg(USB_setup.wValue.b[0], USB_setup.wIndex.b[0]);
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case MPU_READ_REG:
            BD[EP0IN].address[0] = mpu_readReg(USB_setup.wValue.b[0]);
            BD[EP0IN].bytecount = 1;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case MPU_WRITE_REGS:
            if (USB_setup.wLength.w > BUFFER_LENGTH)
                USB_error_flags |= REQUEST_ERROR;
            else {
                USB_request.setup.bmRequestType = USB_setup.bmRequestType;
                USB_request.setup.bRequest = USB_setup.bRequest;
                USB_request.setup.wValue.w = USB_setup.wValue.w;
                USB_request.setup.wIndex.w = USB_setup.wIndex.w;
                USB_request.setup.wLength.w = USB_setup.wLength.w;
                USB_request.bytes_left.w = USB_setup.wLength.w;
                USB_request.data_ptr = receive_buffer;
                USB_request.done_callback = write_regs_from_buffer;
                USB_out_callbacks[0] = usb_receive_data_packet;
            }
            break;
        case MPU_READ_REGS:
            mpu_readRegs(USB_setup.wValue.b[0], BD[EP0IN].address, USB_setup.wLength.b[0]);
            BD[EP0IN].bytecount = USB_setup.wLength.b[0];
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case MPU_SET_ACCEL_SCALE:
            mpu_set_accel_scale(USB_setup.wValue.w);
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case MPU_GET_ACCEL_SCALE:
            BD[EP0IN].address[0] = (uint8_t)mpu_get_accel_scale();
            BD[EP0IN].bytecount = 1;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case MPU_SET_GYRO_SCALE:
            mpu_set_gyro_scale(USB_setup.wValue.w);
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case MPU_GET_GYRO_SCALE:
            BD[EP0IN].address[0] = (uint8_t)mpu_get_gyro_scale();
            BD[EP0IN].bytecount = 1;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case MPU_WHOAMI:
            BD[EP0IN].address[0] = (uint8_t)mpu_whoami();
            BD[EP0IN].bytecount = 1;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case MPU_READ_ACCEL:
            mpu_read_accel(&values);
            temp.i = values.x;
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];
            temp.i = values.y;
            BD[EP0IN].address[2] = temp.b[0];
            BD[EP0IN].address[3] = temp.b[1];
            temp.i = values.z;
            BD[EP0IN].address[4] = temp.b[0];
            BD[EP0IN].address[5] = temp.b[1];
            BD[EP0IN].bytecount = 6;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case MPU_READ_GYRO:
            mpu_read_gyro(&values);
            temp.i = values.x;
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];
            temp.i = values.y;
            BD[EP0IN].address[2] = temp.b[0];
            BD[EP0IN].address[3] = temp.b[1];
            temp.i = values.z;
            BD[EP0IN].address[4] = temp.b[0];
            BD[EP0IN].address[5] = temp.b[1];
            BD[EP0IN].bytecount = 6;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case MPU_READ_TEMP:
            temp.i = mpu_read_temp();
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];
            BD[EP0IN].bytecount = 2;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
#ifdef MPU_MAG
        case MPU_MAG_WHOAMI:
            BD[EP0IN].address[0] = (uint8_t)mpu_mag_whoami();
            BD[EP0IN].bytecount = 1;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case MPU_READ_MAG:
            mpu_read_mag(&values);
            temp.i = values.x;
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];
            temp.i = values.y;
            BD[EP0IN].address[2] = temp.b[0];
            BD[EP0IN].address[3] = temp.b[1];
            temp.i = values.z;
            BD[EP0IN].address[4] = temp.b[0];
            BD[EP0IN].address[5] = temp.b[1];
            BD[EP0IN].bytecount = 6;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case MPU_READ_MAG_CAL:
            BD[EP0IN].address[0] = mpu_mag_cal_vals[0];
            BD[EP0IN].address[1] = mpu_mag_cal_vals[1];
            BD[EP0IN].address[2] = mpu_mag_cal_vals[2];
            BD[EP0IN].bytecount = 3;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
#endif
        default:
            USB_error_flags |= REQUEST_ERROR;
    }
}

int16_t main(void) {
    init_elecanisms();
    init_mpu();

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

