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

#ifndef _USB_H_
#define _USB_H_

#include "common.h"

#define USB_INTERRUPT

#define NUM_CONFIGURATIONS      1
#define NUM_INTERFACES          1
#define NUM_STRINGS             4
#define MAX_PACKET_SIZE         64      // maximum packet size for low-speed peripherals is 8 bytes, for full-speed peripherals it can be 8, 16, 32, or 64 bytes

// states that the USB interface can be in
#define POWERED_STATE           0x00
#define DEFAULT_STATE           0x01
#define ADDRESS_STATE           0x02
#define CONFIG_STATE            0x03

// values for EP control registers
#define ENDPT_DISABLED          0x00
#define ENDPT_IN_ONLY           0x05
#define ENDPT_OUT_ONLY          0x09
#define ENDPT_CONTROL           0x0D    // enable for in, out and setup
#define ENDPT_NON_CONTROL       0x1D    // enable for in, and out

// values for the PID bits in the BD status register
#define TOKEN_OUT               (0x01 << 2)
#define TOKEN_ACK               (0x02 << 2)
#define TOKEN_IN                (0x09 << 2)
#define TOKEN_SETUP             (0x0D << 2)

// masks for bits in the BD status register
#define UOWN                    0x80
#define DTS                     0x40
#define DTSEN                   0x08    // pertains to CPU mode (not USB mode)
#define BSTALL                  0x04    // pertains to CPU mode (not USB mode)

// standard descriptor types
#define DEVICE                  1
#define CONFIGURATION           2
#define STRING                  3
#define INTERFACE               4
#define ENDPOINT                5

// standard USB requests
#define NO_REQUEST              0xFF
#define GET_STATUS              0x00
#define CLEAR_FEATURE           0x01
#define SET_FEATURE             0x03
#define SET_ADDRESS             0x05
#define GET_DESCRIPTOR          0x06
#define SET_DESCRIPTOR          0x07
#define GET_CONFIGURATION       0x08
#define SET_CONFIGURATION       0x09
#define GET_INTERFACE           0x0A
#define SET_INTERFACE           0x0B
#define SYNCH_FRAME             0x0C

#define STANDARD_REQ            (0x00 << 5)
#define CLASS_REQ               (0x01 << 5)
#define VENDOR_REQ              (0x02 << 5)

#define RECIPIENT_DEVICE        0x00
#define RECIPIENT_INTERFACE     0x01
#define RECIPIENT_ENDPOINT      0x02

#define DEVICE_REMOTE_WAKEUP    0x01
#define ENDPOINT_HALT           0x00

// bit masks to clear flags in the USB OTG Interrupt Status Register (host mode)
#define U1OTGIR_IDIF            0x80
#define U1OTGIR_T1MSECIF        0x40
#define U1OTGIR_LSTATEIF        0x20
#define U1OTGIR_ACTVIF          0x10
#define U1OTGIR_SESVDIF         0x08
#define U1OTGIR_SESENDIF        0x04
#define U1OTGIR_VBUSVDIF        0x01

// bit masks to clear flags in the USB Interrupt Status Register
#define U1IR_STALLIF            0x80
#define U1IR_ATTACHIF           0x40    // pertaints only to host mode
#define U1IR_RESUMEIF           0x20
#define U1IR_IDLEIF             0x10
#define U1IR_TRNIF              0x08
#define U1IR_SOFIF              0x04
#define U1IR_UERRIF             0x02
#define U1IR_URSTIF             0x01

// bit masks to clear flags in the USB Error Interrupt Status Register
#define U1EIR_BTSEF             0x80
#define U1EIR_DMAEF             0x20
#define U1EIR_BTOEF             0x10
#define U1EIR_DFN8EF            0x08
#define U1EIR_CRC16EF           0x04
#define U1EIR_CRC5EF            0x02    // pertains only to device mode
#define U1EIR_EOFEF             0x02    // pertains only to host mode
#define U1EIR_PIDEF             0x01

// offsets into the buffer descriptor table
#define EP0OUT                  0
#define EP0IN                   1
#define EP1OUT                  2
#define EP1IN                   3
#define EP2OUT                  4
#define EP2IN                   5
#define EP3OUT                  6
#define EP3IN                   7
#define EP4OUT                  8
#define EP4IN                   9
#define EP5OUT                  10
#define EP5IN                   11
#define EP6OUT                  12
#define EP6IN                   13
#define EP7OUT                  14
#define EP7IN                   15
#define EP8OUT                  16
#define EP8IN                   17
#define EP9OUT                  18
#define EP9IN                   19
#define EP10OUT                 20
#define EP10IN                  21
#define EP11OUT                 22
#define EP11IN                  23
#define EP12OUT                 24
#define EP12IN                  25
#define EP13OUT                 26
#define EP13IN                  27
#define EP14OUT                 28
#define EP14IN                  29
#define EP15OUT                 30
#define EP15IN                  31

// Bit masks for USB_error_flags
#define REQUEST_ERROR           0x01

typedef struct {
    uint8_t bytecount;
    uint8_t status;
    uint8_t *address;
} BUFDESC;

typedef struct {
    uint8_t bmRequestType;
    uint8_t bRequest;
    WORD wValue;
    WORD wIndex;
    WORD wLength;
} SETUP;

typedef void (*USB_CALLBACK_T)(void);

typedef struct {
    SETUP setup;
    uint8_t *data_ptr;
    WORD bytes_left;
    USB_CALLBACK_T done_callback;
} REQUEST;

extern __attribute__ ((aligned (512))) BUFDESC BD[];

extern BUFDESC USB_buffer_desc;
extern SETUP USB_setup;
extern REQUEST USB_request;
extern uint8_t USB_error_flags;
extern uint8_t USB_curr_config;
extern uint8_t USB_device_status;
extern uint8_t USB_USTAT;
extern uint8_t USB_USWSTAT;

extern USB_CALLBACK_T USB_set_config_callback;
extern USB_CALLBACK_T USB_get_descriptor_callback;
extern USB_CALLBACK_T USB_setup_class_callback;
extern USB_CALLBACK_T USB_setup_vendor_callback;
extern USB_CALLBACK_T USB_in_callbacks[];
extern USB_CALLBACK_T USB_out_callbacks[];

extern uint8_t __attribute__ ((space(auto_psv))) Device[];
extern uint8_t __attribute__ ((space(auto_psv))) *Configurations[];
extern uint8_t __attribute__ ((space(auto_psv))) *Strings[];

extern void init_usb(void);
extern void usb_service(void);
extern void usb_send_data_packet(void);
extern void usb_receive_data_packet(void);

#endif
