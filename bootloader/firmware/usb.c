#include <p24FJ128GB206.h>
#include "usb.h"

BUFDESC __attribute__ ((aligned (512))) BD[32];

BYTE EP0_OUT_buffer[MAX_PACKET_SIZE];
BYTE EP0_IN_buffer[MAX_PACKET_SIZE];

BUFDESC USB_buffer_desc;
SETUP USB_setup;
REQUEST USB_request;
BYTE USB_error_flags;
BYTE USB_curr_config;
BYTE USB_device_status;
BYTE USB_USTAT;
BYTE USB_USWSTAT;

void InitUSB(void) {
    unsigned int *U1EP;
    BYTE n;

    U1CONbits.PPBRST = 1;
    U1ADDR = 0;
    U1EIR = 0xFF;
    U1CONbits.PPBRST = 0;
    U1EP = (unsigned int *)&U1EP0;
    for (n = 0; n<16; n++)
        U1EP[n] = ENDPT_DISABLED;
    U1EP0 = ENDPT_CONTROL;
    BD[EP0OUT].bytecount = MAX_PACKET_SIZE;
    BD[EP0OUT].address = EP0_OUT_buffer;    // EP0 OUT gets a buffer
    BD[EP0OUT].status = 0x88;               // set UOWN bit (USB can write)
    BD[EP0IN].address = EP0_IN_buffer;      // EP0 IN gets a buffer
    BD[EP0IN].status = 0x08;                // clear UOWN bit (MCU can write)
    U1CNFG1 = 0x00;
    U1CNFG2 = 0x00;
    U1BDTP1 = (unsigned int)BD>>8;
    U1OTGCONbits.OTGEN = 1;
    U1OTGCONbits.DPPULUP = 1;
    U1PWRCbits.USBPWR = 1;
    U1CONbits.PKTDIS = 0;
    USB_curr_config = 0x00;
    USB_USWSTAT = 0x00;         // default to powered state
    USB_device_status = 0x01;
    USB_request.setup.bmRequestType = NO_REQUEST;    // No device requests in process
    USB_request.setup.bRequest = NO_REQUEST;
#ifdef SHOW_ENUM_STATUS
    TRISB = 0x00;               // set all bits of PORTB as outputs
    PORTB = 0x01;               // set bit zero to indicate Powered status
#endif
    while (U1CONbits.SE0) {}
}

void ServiceUSB(void) {
    BUFDESC *buf_desc_ptr;
    unsigned int *U1EP;
    BYTE n;

    if (U1IRbits.UERRIF) {
        U1EIR = 0xFF;           // clear all flags in U1EIR to clear U1EIR
        U1IR = U1IR_UERRIF;     // clear UERRIF
    } else if (U1IRbits.SOFIF) {
        U1IR = U1IR_SOFIF;      // clear SOFIF
    } else if (U1IRbits.IDLEIF) {
        U1IR = U1IR_IDLEIF;     // clear IDLEIF
//      U1PWRCbits.USUSPND = 1; // put USB module in suspend mode
#ifdef SHOW_ENUM_STATUS
        PORTB &= 0xE0;
        PORTBbits.RB4 = 1;
#endif
    } else if (U1IRbits.RESUMEIF) {
        U1IR = U1IR_RESUMEIF;   // clear RESUMEIF
//      U1PWRCbits.USUSPND = 0; // resume USB module operation
#ifdef SHOW_ENUM_STATUS
        PORTB &= 0xE0;
        PORTB |= 0x01<<USB_USWSTAT;
#endif
    } else if (U1IRbits.STALLIF) {
        U1IR = U1IR_STALLIF;    // clear STALLIF
    } else if (U1IRbits.URSTIF) {
        USB_curr_config = 0x00;
        while (U1IRbits.TRNIF) {
            U1IR = U1IR_TRNIF;  // clear TRNIF to advance the U1STAT FIFO
            __builtin_nop();    // wait for six instruction cycles...
            __builtin_nop();
            __builtin_nop();
            __builtin_nop();
            __builtin_nop();
            __builtin_nop();    // ...to allow a pending TRNIF to appear
        }
        U1EP = (unsigned int *)&U1EP0;
        for (n = 0; n<16; n++)
            U1EP[n] = ENDPT_DISABLED;   // clear all EP control registers to disable all endpoints
        BD[EP0OUT].bytecount = MAX_PACKET_SIZE;
        BD[EP0OUT].address = EP0_OUT_buffer;    // EP0 OUT gets a buffer
        BD[EP0OUT].status = 0x88;               // set UOWN bit (USB can write)
        BD[EP0IN].address = EP0_IN_buffer;      // EP0 IN gets a buffer
        BD[EP0IN].status = 0x08;                // clear UOWN bit (MCU can write)
        U1ADDR = 0x00;                  // set USB Address to 0
        U1IR = 0xFF;                    // clear all the USB interrupt flags
        U1EP0 = ENDPT_CONTROL;          // EP0 is a control pipe and requires an ACK
//      U1EIE = 0x00FF;                 // enable all USB error interrupts
        USB_USWSTAT = DEFAULT_STATE;
        USB_device_status = 0x01;       // self powered, remote wakeup disabled
#ifdef SHOW_ENUM_STATUS
        PORTB &= 0xE0;
        PORTBbits.RB1 = 1;              // set bit 1 of PORTB to indicate Powered state
#endif
    } else if (U1IRbits.TRNIF) {
        buf_desc_ptr = &BD[U1STAT>>3];  // ENDPT and DIR bits of U1STAT provide the offset into the buffer descriptor table
        USB_buffer_desc.status = buf_desc_ptr->status;
        USB_buffer_desc.bytecount = buf_desc_ptr->bytecount;
        USB_buffer_desc.address = buf_desc_ptr->address;
        USB_USTAT = U1STAT;             // save the USB status register
        U1IR = U1IR_TRNIF;              // clear TRNIF
#ifdef SHOW_ENUM_STATUS
        if (USB_USTAT&0xF0==EP0) {      // toggle RB5 to reflect EP0 activity
            PORTB ^= 0x20;
        }
#endif
        USB_error_flags = 0x00;         // clear USB error flags
        switch (USB_buffer_desc.status&0x3C) {    // extract PID bits
            case TOKEN_SETUP:
                ProcessSetupToken();
                break;
            case TOKEN_IN:
                ProcessInToken();
                break;
            case TOKEN_OUT:
                ProcessOutToken();
        }
        if (USB_error_flags&0x01) {             // if there was a Request Error...
            BD[EP0OUT].bytecount = MAX_PACKET_SIZE;   // ...get ready to receive the next Setup token...
            BD[EP0IN].status = 0x84;
            BD[EP0OUT].status = 0x84;                 // ...and issue a protocol stall on EP0
        }
    }
}

void ProcessSetupToken(void) {
    BYTE *buf_ptr;

    buf_ptr = USB_buffer_desc.address;
    USB_setup.bmRequestType = *buf_ptr++;   // copy the setup packet received from the host
    USB_setup.bRequest = *buf_ptr++;
    USB_setup.wValue.b[0] = *buf_ptr++;
    USB_setup.wValue.b[1] = *buf_ptr++;
    USB_setup.wIndex.b[0] = *buf_ptr++;
    USB_setup.wIndex.b[1] = *buf_ptr++;
    USB_setup.wLength.b[0] = *buf_ptr++;
    USB_setup.wLength.b[1] = *buf_ptr++;
    BD[EP0OUT].bytecount = MAX_PACKET_SIZE;     // reset the EP0 OUT byte count
    BD[EP0IN].status = 0x08;                   // return the EP0 IN buffer to us (dequeue any pending requests)
    BD[EP0OUT].status = (!(USB_setup.bmRequestType&0x80) && (USB_setup.wLength.w)) ? 0xC8:0x88;    // set EP0 OUT UOWN back to USB and DATA0/DATA1 packet according to the request type
    U1CONbits.PKTDIS = 0;                 // assuming there is nothing to dequeue, clear the packet disable bit
    USB_request.setup.bmRequestType = NO_REQUEST;   // clear the device request in process
    USB_request.setup.bRequest = NO_REQUEST;
    switch (USB_setup.bmRequestType&0x60) {    // extract request type bits
        case STANDARD_REQ:
            StandardRequests();
            break;
        case VENDOR_REQ:
            VendorRequests();
            break;
        case CLASS_REQ:
//            ClassRequests();
//            break;
        default:
            USB_error_flags |= 0x01;    // set Request Error Flag
    }
}

void StandardRequests(void) {
    BUFDESC *buf_desc_ptr;
    unsigned int *U1EP;
    BYTE n;

    switch (USB_setup.bRequest) {
        case GET_STATUS:
            switch (USB_setup.bmRequestType&0x1F) { // extract request recipient bits
                case RECIPIENT_DEVICE:
                    BD[EP0IN].address[0] = USB_device_status;
                    BD[EP0IN].address[1] = 0x00;
                    BD[EP0IN].bytecount = 0x02;
                    BD[EP0IN].status = 0xC8;                     // send packet as DATA1, set UOWN bit
                    break;
                case RECIPIENT_INTERFACE:
                    switch (USB_USWSTAT) {
                        case ADDRESS_STATE:
                            USB_error_flags |= 0x01;        // set Request Error Flag
                            break;
                        case CONFIG_STATE:
                            if (USB_setup.wIndex.b[0]<NUM_INTERFACES) {
                                BD[EP0IN].address[0] = 0x00;
                                BD[EP0IN].address[1] = 0x00;
                                BD[EP0IN].bytecount = 0x02;
                                BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
                            } else {
                                USB_error_flags |= 0x01;    // set Request Error Flag
                            }
                    }
                    break;
                case RECIPIENT_ENDPOINT:
                    switch (USB_USWSTAT) {
                        case ADDRESS_STATE:
                            if (!(USB_setup.wIndex.b[0]&0x0F)) {    // get EP, strip off direction bit and see if it is EP0
                                BD[EP0IN].address[0] = (((USB_setup.wIndex.b[0]&0x80) ? BD[EP0IN].status:BD[EP0OUT].status)&0x04)>>2;    // return the BSTALL bit of EP0 IN or OUT, whichever was requested
                                BD[EP0IN].address[1] = 0x00;
                                BD[EP0IN].bytecount = 0x02;
                                BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
                            } else {
                                USB_error_flags |= 0x01;    // set Request Error Flag
                            }
                            break;
                        case CONFIG_STATE:
                            U1EP = (unsigned int *)&U1EP0;
                            n = USB_setup.wIndex.b[0]&0x0F;    // get EP and strip off direction bit for offset from U1EP0
                            buf_desc_ptr = &BD[EP0OUT]+((n<<1)|((USB_setup.wIndex.b[0]&0x80) ? 0x01:0x00));    // compute pointer to the buffer descriptor for the specified EP
                            if (U1EP[n]&((USB_setup.wIndex.b[0]&0x80) ? 0x04:0x08)) { // if the specified EP is enabled for transfers in the specified direction...
                                BD[EP0IN].address[0] = ((buf_desc_ptr->status)&0x04)>>2;    // ...return the BSTALL bit of the specified EP
                                BD[EP0IN].address[1] = 0x00;
                                BD[EP0IN].bytecount = 0x02;
                                BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
                            } else {
                                USB_error_flags |= 0x01;    // set Request Error Flag
                            }
                            break;
                        default:
                            USB_error_flags |= 0x01;        // set Request Error Flag
                    }
                    break;
                default:
                    USB_error_flags |= 0x01;                // set Request Error Flag
            }
            break;
        case CLEAR_FEATURE:
        case SET_FEATURE:
            switch (USB_setup.bmRequestType&0x1F) {         // extract request recipient bits
                case RECIPIENT_DEVICE:
                    switch (USB_setup.wValue.b[0]) {
                        case DEVICE_REMOTE_WAKEUP:
                            if (USB_setup.bRequest==CLEAR_FEATURE)
                                USB_device_status &= 0xFE;
                            else
                                USB_device_status |= 0x01;
                            BD[EP0IN].bytecount = 0x00;          // set EP0 IN byte count to 0
                            BD[EP0IN].status = 0xC8;             // send packet as DATA1, set UOWN bit
                            break;
                        default:
                            USB_error_flags |= 0x01;        // set Request Error Flag
                    }
                    break;
                case RECIPIENT_ENDPOINT:
                    switch (USB_USWSTAT) {
                        case ADDRESS_STATE:
                            if (!(USB_setup.wIndex.b[0]&0x0F)) {    // get EP, strip off direction bit, and see if its EP0
                                BD[EP0IN].bytecount = 0x00;      // set EP0 IN byte count to 0
                                BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
                            } else {
                                USB_error_flags |= 0x01;    // set Request Error Flag
                            }
                            break;
                        case CONFIG_STATE:
                            U1EP = (unsigned int *)&U1EP0;
                            if (n = USB_setup.wIndex.b[0]&0x0F) {    // get EP and strip off direction bit for offset from U1EP0, if not EP0...
                                buf_desc_ptr = &BD[(n<<1)|((USB_setup.wIndex.b[0]&0x80) ? 0x01:0x00)];    // compute pointer to the buffer descriptor for the specified EP
                                if (USB_setup.wIndex.b[0]&0x80) {    // if the specified EP direction is IN...
                                    if (U1EP[n]&0x04) {     // if EPn is enabled for IN transfers...
                                        buf_desc_ptr->status = (USB_setup.bRequest==CLEAR_FEATURE) ? 0x00:0x84;
                                    } else {
                                        USB_error_flags |= 0x01;    // set Request Error Flag
                                    }
                                } else {                    // ...otherwise the specified EP direction is OUT, so...
                                    if (U1EP[n]&0x08) {     // if EPn is enabled for OUT transfers...
                                        buf_desc_ptr->status = (USB_setup.bRequest==CLEAR_FEATURE) ? 0x88:0x84;
                                    } else {
                                        USB_error_flags |= 0x01;    // set Request Error Flag
                                    }
                                }
                            }
                            if (!(USB_error_flags&0x01)) {  // if there was no Request Error...
                                BD[EP0IN].bytecount = 0x00;
                                BD[EP0IN].status = 0xC8;         // ...send packet as DATA1, set UOWN bit
                            }
                            break;
                        default:
                            USB_error_flags |= 0x01;        // set Request Error Flag
                    }
                    break;
                default:
                    USB_error_flags |= 0x01;                // set Request Error Flag
            }
            break;
        case SET_ADDRESS:
            if (USB_setup.wValue.b[0]>0x7F) {       // if new device address is illegal, send Request Error
                USB_error_flags |= 0x01;            // set Request Error Flag
            } else {
                USB_request.setup.bmRequestType = USB_setup.bmRequestType;  // processing a SET_ADDRESS request
                USB_request.setup.bRequest = USB_setup.bRequest;
                USB_request.setup.wValue.w = USB_setup.wValue.w;
                USB_request.setup.wIndex.w = USB_setup.wIndex.w;
                USB_request.setup.wLength.w = USB_setup.wLength.w;
                BD[EP0IN].bytecount = 0x00;              // set EP0 IN byte count to 0
                BD[EP0IN].status = 0xC8;                 // send packet as DATA1, set UOWN bit
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
                    USB_request.bytes_left.w = (unsigned int)USB_request.data_ptr[0];
                    if (USB_setup.wLength.w<USB_request.bytes_left.w) {
                        USB_request.bytes_left.w = USB_setup.wLength.w;
                    }
                    SendDataPacket();
                    break;
                case CONFIGURATION:
                    switch (USB_setup.wValue.b[0]) {
                        case 0:
                            USB_request.data_ptr = Configuration1;
                            break;
                        default:
                            USB_error_flags |= 0x01;    // set Request Error Flag
                    }
                    if (!(USB_error_flags&0x01)) {
                        USB_request.bytes_left.b[0] = USB_request.data_ptr[2];    // wTotalLength starting at an offset of 2
                        USB_request.bytes_left.b[1] = USB_request.data_ptr[3];
                        if (USB_setup.wLength.w<USB_request.bytes_left.w) {
                            USB_request.bytes_left.w = USB_setup.wLength.w;
                        }
                        SendDataPacket();
                    }
                    break;
                case STRING:
                    switch (USB_setup.wValue.b[0]) {
                        case 0:
                            USB_request.data_ptr = String0;
                            break;
                        case 1:
                            USB_request.data_ptr = String1;
                            break;
                        case 2:
                            USB_request.data_ptr = String2;
                            break;
                        default:
                            USB_error_flags |= 0x01;    // set Request Error Flag
                    }
                    if (!(USB_error_flags&0x01)) {
                        USB_request.bytes_left.w = (unsigned int)USB_request.data_ptr[0];
                        if (USB_setup.wLength.w<USB_request.bytes_left.w) {
                            USB_request.bytes_left.w = USB_setup.wLength.w;
                        }
                        SendDataPacket();
                    }
                    break;
                default:
                    USB_error_flags |= 0x01;            // set Request Error Flag
            }
            break;
        case GET_CONFIGURATION:
            BD[EP0IN].address[0] = USB_curr_config;          // copy current device configuration to EP0 IN buffer
            BD[EP0IN].bytecount = 0x01;
            BD[EP0IN].status = 0xC8;                         // send packet as DATA1, set UOWN bit
            break;
        case SET_CONFIGURATION:
            if (USB_setup.wValue.b[0]<=NUM_CONFIGURATIONS) {
                U1EP = (unsigned int *)&U1EP0;
                for (n = 1; n<16; n++)
                    U1EP[n] = ENDPT_DISABLED;   // clear all EP control registers except for EP0 to disable EP1-EP15 prior to setting configuration
                switch (USB_curr_config = USB_setup.wValue.b[0]) {
                    case 0:
                        USB_USWSTAT = ADDRESS_STATE;
#ifdef SHOW_ENUM_STATUS
                        PORTB &= 0xE0;
                        PORTBbits.RB2 = 1;
#endif
                        break;
                    default:
                        USB_USWSTAT = CONFIG_STATE;
#ifdef SHOW_ENUM_STATUS
                        PORTB &= 0xE0;
                        PORTBbits.RB3 = 1;
#endif
                }
                BD[EP0IN].bytecount = 0x00;                  // set EP0 IN byte count to 0
                BD[EP0IN].status = 0xC8;                     // send packet as DATA1, set UOWN bit
            } else {
                USB_error_flags |= 0x01;                // set Request Error Flag
            }
            break;
        case GET_INTERFACE:
            switch (USB_USWSTAT) {
                case CONFIG_STATE:
                    if (USB_setup.wIndex.b[0]<NUM_INTERFACES) {
                        BD[EP0IN].address[0] = 0x00;         // always send back 0 for bAlternateSetting
                        BD[EP0IN].bytecount = 0x01;
                        BD[EP0IN].status = 0xC8;             // send packet as DATA1, set UOWN bit
                    } else {
                        USB_error_flags |= 0x01;        // set Request Error Flag
                    }
                    break;
                default:
                    USB_error_flags |= 0x01;            // set Request Error Flag
            }
            break;
        case SET_INTERFACE:
            switch (USB_USWSTAT) {
                case CONFIG_STATE:
                    if (USB_setup.wIndex.b[0]<NUM_INTERFACES) {
                        switch (USB_setup.wValue.b[0]) {
                            case 0:                     // currently support only bAlternateSetting of 0
                                BD[EP0IN].bytecount = 0x00;  // set EP0 IN byte count to 0
                                BD[EP0IN].status = 0xC8;     // send packet as DATA1, set UOWN bit
                                break;
                            default:
                                USB_error_flags |= 0x01;    // set Request Error Flag
                        }
                    } else {
                        USB_error_flags |= 0x01;        // set Request Error Flag
                    }
                    break;
                default:
                    USB_error_flags |= 0x01;            // set Request Error Flag
            }
            break;
        case SET_DESCRIPTOR:
        case SYNCH_FRAME:
        default:
            USB_error_flags |= 0x01;                    // set Request Error Flag
    }
}

void ProcessInToken(void) {
    switch (USB_USTAT&0xF0) {    // extract the EP bits
        case EP0:
            switch (USB_request.setup.bmRequestType&0x60) {   // extract request type bits
                case STANDARD_REQ:
                    switch (USB_request.setup.bRequest) {
                        case SET_ADDRESS:
                            switch (U1ADDR = USB_request.setup.wValue.b[0]) {
                                case 0:
                                    USB_USWSTAT = DEFAULT_STATE;
#ifdef SHOW_ENUM_STATUS
                                    PORTB &= 0xE0;
                                    PORTBbits.RB1 = 1;
#endif
                                    break;
                                  default:
                                    USB_USWSTAT = ADDRESS_STATE;
#ifdef SHOW_ENUM_STATUS
                                    PORTB &= 0xE0;
                                    PORTBbits.RB2 = 1;
#endif
                            }
                            break;
                        case GET_DESCRIPTOR:
                            SendDataPacket();
                            break;
                    }
                    break;
                case CLASS_REQ:
                    break;
                case VENDOR_REQ:
                    VendorRequestsIn();
                    break;
            }
            break;
    }
}

void ProcessOutToken(void) {
    switch (USB_USTAT&0xF0) {    // extract the EP bits
        case EP0:
            switch (USB_request.setup.bmRequestType&0x60) {   // extract request type bits
                case STANDARD_REQ:
                    break;
                case CLASS_REQ:
                    break;
                case VENDOR_REQ:
                    VendorRequestsOut();
                    break;
            }
            BD[EP0OUT].bytecount = MAX_PACKET_SIZE;
            BD[EP0OUT].status = 0x88;
            BD[EP0IN].bytecount = 0x00;      // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;         // send packet as DATA1, set UOWN bit
            break;
    }
}

void SendDataPacket(void) {
    BYTE packet_length, n;

    if (USB_request.bytes_left.w<MAX_PACKET_SIZE) {
        packet_length = (BYTE)USB_request.bytes_left.w;
        USB_request.bytes_left.w = 0;
        USB_request.setup.bmRequestType = NO_REQUEST;    // sending a short packet, so clear device request
        USB_request.setup.bRequest = NO_REQUEST;
    } else {
        packet_length = MAX_PACKET_SIZE;
        USB_request.bytes_left.w -= MAX_PACKET_SIZE;
    }
    for (n = 0; n<packet_length; n++) {
        BD[EP0IN].address[n] = *USB_request.data_ptr++;
    }
    BD[EP0IN].bytecount = packet_length;
    BD[EP0IN].status = ((BD[EP0IN].status^0x40)&0x40)|0x88; // toggle the DATA01 bit, clear the PIDs bits, and set the UOWN and DTS bits
}
