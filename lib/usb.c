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

#include "pic24fj.h"
#include "usb.h"

BUFDESC __attribute__ ((aligned (512))) BD[32];

uint8_t EP0_OUT_buffer[MAX_PACKET_SIZE];
uint8_t EP0_IN_buffer[MAX_PACKET_SIZE];

BUFDESC USB_buffer_desc;
SETUP USB_setup;
REQUEST USB_request;
uint8_t USB_error_flags;
uint8_t USB_curr_config;
uint8_t USB_device_status;
uint8_t USB_USTAT;
uint8_t USB_USWSTAT;

USB_CALLBACK_T USB_set_config_callback = (USB_CALLBACK_T)NULL;
USB_CALLBACK_T USB_get_descriptor_callback = (USB_CALLBACK_T)NULL;
USB_CALLBACK_T USB_setup_class_callback = (USB_CALLBACK_T)NULL;
USB_CALLBACK_T USB_setup_vendor_callback = (USB_CALLBACK_T)NULL;
USB_CALLBACK_T USB_in_callbacks[16];
USB_CALLBACK_T USB_out_callbacks[16];

void usb_standard_requests(void);
void usb_process_setup_token(void);

#ifdef USB_INTERRUPT
void __attribute__((interrupt, auto_psv)) _USB1Interrupt(void) {
    usb_service();
}
#endif

void usb_disable_endpoints(uint8_t starting_ep) {
    uint16_t *U1EP;
    uint8_t ep;

    U1EP = (uint16_t *)&U1EP0;
    for (ep = starting_ep; ep < 16; ep++) {
        U1EP[ep] = ENDPT_DISABLED;
        USB_in_callbacks[ep] = (USB_CALLBACK_T)NULL;
        USB_out_callbacks[ep] = (USB_CALLBACK_T)NULL;
    }
}

void usb_set_address_callback(void) {
    switch (U1ADDR = USB_request.setup.wValue.b[0]) {
        case 0:
            USB_USWSTAT = DEFAULT_STATE;
            break;
        default:
            USB_USWSTAT = ADDRESS_STATE;
    }
    USB_in_callbacks[0] = (USB_CALLBACK_T)NULL;
}

void init_usb(void) {
    IEC5bits.USB1IE = 0;                    // disable USB interrupt

    U1CONbits.PPBRST = 1;
    U1ADDR = 0;
    U1EIR = 0xFF;
    U1CONbits.PPBRST = 0;
    usb_disable_endpoints(0);               // disable all endpoints
    U1EP0 = ENDPT_CONTROL;
    BD[EP0OUT].bytecount = MAX_PACKET_SIZE;
    BD[EP0OUT].address = EP0_OUT_buffer;    // EP0 OUT gets a buffer
    BD[EP0OUT].status = UOWN | DTSEN;       // set UOWN bit (USB can write)
    BD[EP0IN].address = EP0_IN_buffer;      // EP0 IN gets a buffer
    BD[EP0IN].status = DTSEN;               // clear UOWN bit (MCU can write)
    U1CNFG1 = 0;
    U1CNFG2 = 0;
    U1BDTP1 = (uint16_t)BD >> 8;
    U1OTGCONbits.OTGEN = 1;
    U1OTGCONbits.DPPULUP = 1;
    U1PWRCbits.USBPWR = 1;
    U1CONbits.PKTDIS = 0;
    USB_curr_config = 0;
    USB_USWSTAT = POWERED_STATE;
    USB_device_status = 1;
    USB_request.setup.bmRequestType = NO_REQUEST;
    USB_request.setup.bRequest = NO_REQUEST;
    USB_request.bytes_left.w = 0;
    USB_request.done_callback = (USB_CALLBACK_T)NULL;
    while (U1CONbits.SE0) {}

#ifdef USB_INTERRUPT
    U1IE = 0xFF;
    U1EIE = 0xFF;
    IFS5bits.USB1IF = 0;
    IEC5bits.USB1IE = 1;
#endif
}

void usb_service(void) {
    BUFDESC *buf_desc_ptr;
    uint8_t ep;

    if (U1IRbits.UERRIF) {
        U1EIR = 0xFF;                       // clear all flags in U1EIR to clear U1EIR
        U1IR = U1IR_UERRIF;                 // clear UERRIF
    } else if (U1IRbits.SOFIF) {
        U1IR = U1IR_SOFIF;                  // clear SOFIF
    } else if (U1IRbits.IDLEIF) {
        U1IR = U1IR_IDLEIF;                 // clear IDLEIF
//      U1PWRCbits.USUSPND = 1;             // put USB module in suspend mode
    } else if (U1IRbits.RESUMEIF) {
        U1IR = U1IR_RESUMEIF;               // clear RESUMEIF
//      U1PWRCbits.USUSPND = 0;             // resume USB module operation
    } else if (U1IRbits.STALLIF) {
        U1IR = U1IR_STALLIF;                // clear STALLIF
    } else if (U1IRbits.URSTIF) {
        USB_curr_config = 0;
        while (U1IRbits.TRNIF) {
            U1IR = U1IR_TRNIF;              // clear TRNIF to advance the U1STAT FIFO
            __builtin_nop();                // wait for six instruction cycles...
            __builtin_nop();
            __builtin_nop();
            __builtin_nop();
            __builtin_nop();
            __builtin_nop();                // ...to allow a pending TRNIF to appear
        }
        usb_disable_endpoints(0);           // disable all endpoints
        BD[EP0OUT].bytecount = MAX_PACKET_SIZE;
        BD[EP0OUT].address = EP0_OUT_buffer;    // EP0 OUT gets a buffer
        BD[EP0OUT].status = UOWN | DTSEN;   // set UOWN bit (USB can write)
        BD[EP0IN].address = EP0_IN_buffer;  // EP0 IN gets a buffer
        BD[EP0IN].status = DTSEN;           // clear UOWN bit (MCU can write)
        U1ADDR = 0;                         // set USB Address to 0
        U1IR = 0xFF;                        // clear all the USB interrupt flags
        U1EP0 = ENDPT_CONTROL;              // EP0 is a control pipe and requires an ACK
//      U1EIE = 0x00FF;                     // enable all USB error interrupts
        USB_USWSTAT = DEFAULT_STATE;
        USB_device_status = 1;              // self powered, remote wakeup disabled
    } else if (U1IRbits.TRNIF) {
        buf_desc_ptr = &BD[U1STAT >> 3];    // ENDPT and DIR bits of U1STAT provide the offset into the buffer descriptor table
        USB_buffer_desc.status = buf_desc_ptr->status;
        USB_buffer_desc.bytecount = buf_desc_ptr->bytecount;
        USB_buffer_desc.address = buf_desc_ptr->address;
        USB_USTAT = U1STAT;                 // save the USB status register
        U1IR = U1IR_TRNIF;                  // clear TRNIF
        USB_error_flags = 0;                // clear USB error flags
        switch (USB_buffer_desc.status & 0x3C) {    // extract PID bits
            case TOKEN_SETUP:
                usb_process_setup_token();
                break;
            case TOKEN_IN:
                ep = (USB_USTAT >> 4) & 0x0F;
                if (USB_in_callbacks[ep])
                    USB_in_callbacks[ep]();
                break;
            case TOKEN_OUT:
                ep = (USB_USTAT >> 4) & 0x0F;
                if (USB_out_callbacks[ep])
                    USB_out_callbacks[ep]();
                if ((ep == 0) && (USB_request.bytes_left.w == 0)) {
                    BD[EP0OUT].bytecount = MAX_PACKET_SIZE; // reset EP0 OUT buffer
                    BD[EP0OUT].status = UOWN | DTSEN;
                    BD[EP0IN].bytecount = 0;                // complete status phase of control transfer
                    BD[EP0IN].status = UOWN | DTS | DTSEN;  // send packet as DATA1, set UOWN bit
                }
        }
        if (USB_error_flags & REQUEST_ERROR) {      // if there was a Request Error...
            BD[EP0OUT].bytecount = MAX_PACKET_SIZE; // ...get ready to receive the next Setup token...
            BD[EP0IN].status = UOWN | BSTALL;
            BD[EP0OUT].status = UOWN | BSTALL;      // ...and issue a protocol stall on EP0
        }
    }
    IFS5bits.USB1IF = 0;                    // clear USB Interrupt flag
}

void usb_process_setup_token(void) {
    uint8_t *buf_ptr;

    buf_ptr = USB_buffer_desc.address;
    USB_setup.bmRequestType = *buf_ptr++;           // copy the setup packet received from the host
    USB_setup.bRequest = *buf_ptr++;
    USB_setup.wValue.b[0] = *buf_ptr++;
    USB_setup.wValue.b[1] = *buf_ptr++;
    USB_setup.wIndex.b[0] = *buf_ptr++;
    USB_setup.wIndex.b[1] = *buf_ptr++;
    USB_setup.wLength.b[0] = *buf_ptr++;
    USB_setup.wLength.b[1] = *buf_ptr++;
    BD[EP0OUT].bytecount = MAX_PACKET_SIZE;         // reset the EP0 OUT byte count
    BD[EP0IN].status = DTSEN;                       // return the EP0 IN buffer to us (dequeue any pending requests)
    BD[EP0OUT].status = (!(USB_setup.bmRequestType & UOWN) && (USB_setup.wLength.w)) ? UOWN | DTS | DTSEN:0x88;    // set EP0 OUT UOWN back to USB and DATA0/DATA1 packet according to the request type
    U1CONbits.PKTDIS = 0;                           // assuming there is nothing to dequeue, clear the packet disable bit
    USB_request.setup.bmRequestType = NO_REQUEST;   // clear the device request in process
    USB_request.setup.bRequest = NO_REQUEST;
    USB_request.bytes_left.w = 0;
    USB_request.done_callback = (USB_CALLBACK_T)NULL;
    switch (USB_setup.bmRequestType & 0x60) {       // extract request type bits
        case STANDARD_REQ:
            usb_standard_requests();
            break;
        case VENDOR_REQ:
            if (USB_setup_vendor_callback)
                USB_setup_vendor_callback();
            else
                USB_error_flags |= REQUEST_ERROR;
            break;
        case CLASS_REQ:
            if (USB_setup_class_callback)
                USB_setup_class_callback();
            else
                USB_error_flags |= REQUEST_ERROR;
            break;
        default:
            USB_error_flags |= REQUEST_ERROR;
    }
}

void usb_standard_requests(void) {
    BUFDESC *buf_desc_ptr;
    uint16_t *U1EP;
    uint8_t ep;

    switch (USB_setup.bRequest) {
        case GET_STATUS:
            switch (USB_setup.bmRequestType & 0x1F) {   // extract request recipient bits
                case RECIPIENT_DEVICE:
                    BD[EP0IN].address[0] = USB_device_status;
                    BD[EP0IN].address[1] = 0;
                    BD[EP0IN].bytecount = 2;
                    BD[EP0IN].status = UOWN | DTS | DTSEN;  // send packet as DATA1, set UOWN bit
                    break;
                case RECIPIENT_INTERFACE:
                    switch (USB_USWSTAT) {
                        case ADDRESS_STATE:
                            USB_error_flags |= REQUEST_ERROR;
                            break;
                        case CONFIG_STATE:
                            if (USB_setup.wIndex.b[0] < NUM_INTERFACES) {
                                BD[EP0IN].address[0] = 0;
                                BD[EP0IN].address[1] = 0;
                                BD[EP0IN].bytecount = 2;
                                BD[EP0IN].status = UOWN | DTS | DTSEN;  // send packet as DATA1, set UOWN bit
                            } else {
                                USB_error_flags |= REQUEST_ERROR;
                            }
                    }
                    break;
                case RECIPIENT_ENDPOINT:
                    switch (USB_USWSTAT) {
                        case ADDRESS_STATE:
                            if (!(USB_setup.wIndex.b[0] & 0x0F)) {  // get EP, strip off direction bit and see if it is EP0
                                BD[EP0IN].address[0] = (((USB_setup.wIndex.b[0] & 0x80) ? BD[EP0IN].status : BD[EP0OUT].status) & 0x04) >> 2;   // return the BSTALL bit of EP0 IN or OUT, whichever was requested
                                BD[EP0IN].address[1] = 0;
                                BD[EP0IN].bytecount = 2;
                                BD[EP0IN].status = UOWN | DTS | DTSEN;  // send packet as DATA1, set UOWN bit
                            } else {
                                USB_error_flags |= REQUEST_ERROR;
                            }
                            break;
                        case CONFIG_STATE:
                            U1EP = (uint16_t *)&U1EP0;
                            ep = USB_setup.wIndex.b[0] & 0x0F;  // get EP and strip off direction bit for offset from U1EP0
                            buf_desc_ptr = &BD[EP0OUT] + ((ep << 1) | ((USB_setup.wIndex.b[0] & 0x80) ? 0x01 : 0x00));  // compute pointer to the buffer descriptor for the specified EP
                            if (U1EP[ep] & ((USB_setup.wIndex.b[0] & 0x80) ? 0x04 : 0x08)) {    // if the specified EP is enabled for transfers in the specified direction...
                                BD[EP0IN].address[0] = ((buf_desc_ptr->status) & 0x04) >> 2;    // ...return the BSTALL bit of the specified EP
                                BD[EP0IN].address[1] = 0;
                                BD[EP0IN].bytecount = 2;
                                BD[EP0IN].status = UOWN | DTS | DTSEN;  // send packet as DATA1, set UOWN bit
                            } else {
                                USB_error_flags |= REQUEST_ERROR;
                            }
                            break;
                        default:
                            USB_error_flags |= REQUEST_ERROR;
                    }
                    break;
                default:
                    USB_error_flags |= REQUEST_ERROR;
            }
            break;
        case CLEAR_FEATURE:
        case SET_FEATURE:
            switch (USB_setup.bmRequestType & 0x1F) {       // extract request recipient bits
                case RECIPIENT_DEVICE:
                    switch (USB_setup.wValue.b[0]) {
                        case DEVICE_REMOTE_WAKEUP:
                            if (USB_setup.bRequest == CLEAR_FEATURE)
                                USB_device_status &= 0xFE;
                            else
                                USB_device_status |= 0x01;
                            BD[EP0IN].bytecount = 0;
                            BD[EP0IN].status = UOWN | DTS | DTSEN;  // send packet as DATA1, set UOWN bit
                            break;
                        default:
                            USB_error_flags |= REQUEST_ERROR;
                    }
                    break;
                case RECIPIENT_ENDPOINT:
                    switch (USB_USWSTAT) {
                        case ADDRESS_STATE:
                            if (!(USB_setup.wIndex.b[0] & 0x0F)) {  // get EP, strip off direction bit, and see if its EP0
                                BD[EP0IN].bytecount = 0;
                                BD[EP0IN].status = UOWN | DTS | DTSEN;  // send packet as DATA1, set UOWN bit
                            } else {
                                USB_error_flags |= REQUEST_ERROR;
                            }
                            break;
                        case CONFIG_STATE:
                            U1EP = (uint16_t *)&U1EP0;
                            if (ep = USB_setup.wIndex.b[0] & 0x0F) {    // get EP and strip off direction bit for offset from U1EP0, if not EP0...
                                buf_desc_ptr = &BD[(ep << 1) | ((USB_setup.wIndex.b[0] & 0x80) ? 0x01 : 0x00)]; // compute pointer to the buffer descriptor for the specified EP
                                if (USB_setup.wIndex.b[0] & 0x80) { // if the specified EP direction is IN...
                                    if (U1EP[ep] & 0x04) {          // if EPn is enabled for IN transfers...
                                        if (USB_setup.bRequest == CLEAR_FEATURE) {
                                            U1EP[ep] &= 0xFD;       // clear EPSTALL bit of U1EPn register
                                            buf_desc_ptr->status &= ~BSTALL;
                                            buf_desc_ptr->status |= DTS;
                                        } else {
                                            buf_desc_ptr->status |= BSTALL;
                                        }
                                    } else {
                                        USB_error_flags |= REQUEST_ERROR;
                                    }
                                } else {                    // ...otherwise the specified EP direction is OUT, so...
                                    if (U1EP[ep] & 0x08) {  // if the EP is enabled for OUT transfers...
                                        if (USB_setup.bRequest == CLEAR_FEATURE) {
                                            U1EP[ep] &= 0xFD;       // clear EPSTALL bit of U1EPn register
                                            buf_desc_ptr->status &= ~BSTALL;
                                            buf_desc_ptr->status &= ~DTS;
                                        } else {
                                            buf_desc_ptr->status |= BSTALL;
                                        }
                                        buf_desc_ptr->status = (USB_setup.bRequest == CLEAR_FEATURE) ? (UOWN | DTSEN) : (UOWN | BSTALL);
                                    } else {
                                        USB_error_flags |= REQUEST_ERROR;
                                    }
                                }
                            }
                            if (!(USB_error_flags & REQUEST_ERROR)) {   // if there was no Request Error...
                                BD[EP0IN].bytecount = 0;
                                BD[EP0IN].status = UOWN | DTS | DTSEN;  // ...send packet as DATA1, set UOWN bit
                            }
                            break;
                        default:
                            USB_error_flags |= REQUEST_ERROR;
                    }
                    break;
                default:
                    USB_error_flags |= REQUEST_ERROR;
            }
            break;
        case SET_ADDRESS:
            if (USB_setup.wValue.b[0] > 0x7F) {     // if new device address is illegal, send Request Error
                USB_error_flags |= REQUEST_ERROR;
            } else {
                USB_request.setup.bmRequestType = USB_setup.bmRequestType;  // processing a SET_ADDRESS request
                USB_request.setup.bRequest = USB_setup.bRequest;
                USB_request.setup.wValue.w = USB_setup.wValue.w;
                USB_request.setup.wIndex.w = USB_setup.wIndex.w;
                USB_request.setup.wLength.w = USB_setup.wLength.w;
                USB_in_callbacks[0] = usb_set_address_callback;
                BD[EP0IN].bytecount = 0;
                BD[EP0IN].status = UOWN | DTS | DTSEN;  // send packet as DATA1, set UOWN bit
            }
            break;
        case GET_DESCRIPTOR:
            USB_request.setup.bmRequestType = USB_setup.bmRequestType;  // processing a GET_DESCRIPTOR request
            USB_request.setup.bRequest = USB_setup.bRequest;
            USB_request.setup.wValue.w = USB_setup.wValue.w;
            USB_request.setup.wIndex.w = USB_setup.wIndex.w;
            USB_request.setup.wLength.w = USB_setup.wLength.w;
            switch (USB_setup.wValue.b[1]) {
                case DEVICE:
                    USB_request.data_ptr = Device;
                    USB_request.bytes_left.w = (uint16_t)USB_request.data_ptr[0];
                    break;
                case CONFIGURATION:
                    if (USB_setup.wValue.b[0] < NUM_CONFIGURATIONS) {
                        USB_request.data_ptr = Configurations[USB_setup.wValue.b[0]];
                        USB_request.bytes_left.b[0] = USB_request.data_ptr[2];  // wTotalLength starting at an offset of 2
                        USB_request.bytes_left.b[1] = USB_request.data_ptr[3];
                    } else
                        USB_error_flags |= REQUEST_ERROR;
                    break;
                case STRING:
                    if (USB_setup.wValue.b[0] < NUM_STRINGS) {
                        USB_request.data_ptr = Strings[USB_setup.wValue.b[0]];
                        USB_request.bytes_left.w = (uint16_t)USB_request.data_ptr[0];
                    } else
                        USB_error_flags |= REQUEST_ERROR;
                    break;
                default:
                    if (USB_get_descriptor_callback)
                        USB_get_descriptor_callback();
                    else
                        USB_error_flags |= REQUEST_ERROR;
            }
            if (!(USB_error_flags & REQUEST_ERROR)) {
                USB_in_callbacks[0] = usb_send_data_packet;
                if (USB_setup.wLength.w < USB_request.bytes_left.w)
                    USB_request.bytes_left.w = USB_setup.wLength.w;
                usb_send_data_packet();
            }
            break;
        case GET_CONFIGURATION:
            BD[EP0IN].address[0] = USB_curr_config; // copy current device configuration to EP0 IN buffer
            BD[EP0IN].bytecount = 1;
            BD[EP0IN].status = UOWN | DTS | DTSEN;  // send packet as DATA1, set UOWN bit
            break;
        case SET_CONFIGURATION:
            if (USB_setup.wValue.b[0] <= NUM_CONFIGURATIONS) {
                usb_disable_endpoints(1);           // disable all endpoints except EP0
                switch (USB_curr_config = USB_setup.wValue.b[0]) {
                    case 0:
                        USB_USWSTAT = ADDRESS_STATE;
                        break;
                    default:
                        USB_USWSTAT = CONFIG_STATE;
                        if (USB_set_config_callback)
                            USB_set_config_callback();
                }
                BD[EP0IN].bytecount = 0;
                BD[EP0IN].status = UOWN | DTS | DTSEN;  // send packet as DATA1, set UOWN bit
            } else {
                USB_error_flags |= REQUEST_ERROR;
            }
            break;
        case GET_INTERFACE:
            switch (USB_USWSTAT) {
                case CONFIG_STATE:
                    if (USB_setup.wIndex.b[0] < NUM_INTERFACES) {
                        BD[EP0IN].address[0] = 0;   // always send back 0 for bAlternateSetting
                        BD[EP0IN].bytecount = 1;
                        BD[EP0IN].status = UOWN | DTS | DTSEN;  // send packet as DATA1, set UOWN bit
                    } else {
                        USB_error_flags |= REQUEST_ERROR;
                    }
                    break;
                default:
                    USB_error_flags |= REQUEST_ERROR;
            }
            break;
        case SET_INTERFACE:
            switch (USB_USWSTAT) {
                case CONFIG_STATE:
                    if (USB_setup.wIndex.b[0] < NUM_INTERFACES) {
                        switch (USB_setup.wValue.b[0]) {
                            case 0:                 // currently support only bAlternateSetting of 0
                                BD[EP0IN].bytecount = 0;
                                BD[EP0IN].status = UOWN | DTS | DTSEN;  // send packet as DATA1, set UOWN bit
                                break;
                            default:
                                USB_error_flags |= REQUEST_ERROR;
                        }
                    } else {
                        USB_error_flags |= REQUEST_ERROR;
                    }
                    break;
                default:
                    USB_error_flags |= REQUEST_ERROR;
            }
            break;
        case SET_DESCRIPTOR:
        case SYNCH_FRAME:
        default:
            USB_error_flags |= REQUEST_ERROR;
    }
}

void usb_send_data_packet(void) {
    uint8_t packet_length, i;

    if (USB_request.bytes_left.w < MAX_PACKET_SIZE) {
        packet_length = (uint8_t)USB_request.bytes_left.w;
        USB_request.bytes_left.w = 0;
    } else {
        packet_length = MAX_PACKET_SIZE;
        USB_request.bytes_left.w -= MAX_PACKET_SIZE;
    }
    for (i = 0; i < packet_length; i++) {
        BD[EP0IN].address[i] = *USB_request.data_ptr++;
    }
    BD[EP0IN].bytecount = packet_length;
    BD[EP0IN].status = ((BD[EP0IN].status ^ DTS) & DTS) | UOWN | DTSEN; // toggle the DATA01 bit, clear the PIDs bits, and set the UOWN and DTS bits
    if (USB_request.bytes_left.w == 0) {
        if (USB_request.done_callback)
            USB_request.done_callback();
        USB_request.setup.bmRequestType = NO_REQUEST;    // sending a short packet, so clear device request
        USB_request.setup.bRequest = NO_REQUEST;
        USB_request.done_callback = (USB_CALLBACK_T)NULL;
        USB_in_callbacks[0] = (USB_CALLBACK_T)NULL;
    }
}

void usb_receive_data_packet(void) {
    uint8_t i;

    for (i = 0; i < BD[EP0OUT].bytecount; i++) {
        *USB_request.data_ptr++ = BD[EP0OUT].address[i];
    }
    USB_request.bytes_left.w -= BD[EP0OUT].bytecount;
    if (USB_request.bytes_left.w > 0) {
        BD[EP0OUT].bytecount = MAX_PACKET_SIZE;
        BD[EP0OUT].status = ((BD[EP0OUT].status ^ DTS) & DTS) | UOWN | DTSEN;   // toggle the DATA01 bit, clear the PIDs bits, and set the UOWN and DTS bits
    } else {
        if (USB_request.done_callback)
            USB_request.done_callback();
        USB_request.setup.bmRequestType = NO_REQUEST;    // all data received, so clear device request
        USB_request.setup.bRequest = NO_REQUEST;
        USB_request.done_callback = (USB_CALLBACK_T)NULL;
        USB_out_callbacks[0] = (USB_CALLBACK_T)NULL;
    }
}
