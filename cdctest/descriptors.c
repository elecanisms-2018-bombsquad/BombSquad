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
#include <stdint.h>

uint8_t __attribute__ ((space(auto_psv))) Device[] = {
    0x12,       // bLength
    DEVICE,     // bDescriptorType
    0x00,       // bcdUSB (low byte)
    0x02,       // bcdUSB (high byte)
    0x02,       // bDeviceClass
    0x00,       // bDeviceSubClass
    0x00,       // bDeviceProtocol
    MAX_PACKET_SIZE,    // bMaxPacketSize
    0x66,       // idVendor (low byte)
    0x66,       // idVendor (high byte)
    0xDC,       // idProduct (low byte)
    0x0C,       // idProduct (high byte)
    0x00,       // bcdDevice (low byte)
    0x00,       // bcdDevice (high byte)
    0x01,       // iManufacturer
    0x02,       // iProduct
    0x00,       // iSerialNumber (none)
    0x01,       // bNumConfigurations
};

uint8_t __attribute__ ((space(auto_psv))) Configuration1[] = {
    0x09,       // bLength
    CONFIGURATION,    // bDescriptorType
    0x43,       // wTotalLength (low byte)
    0x00,       // wTotalLength (high byte)
    NUM_INTERFACES, // bNumInterfaces
    0x01,       // bConfigurationValue
    0x00,       // iConfiguration (none)
    0xA0,       // bmAttributes
    0x32,       // bMaxPower (100 mA)
    0x09,       // bLength (Interface0 descriptor starts here)
    INTERFACE,  // bDescriptorType
    0x00,       // bInterfaceNumber
    0x00,       // bAlternateSetting
    0x01,       // bNumEndpoints (excluding EP0)
    0x02,       // bInterfaceClass (Com interface)
    0x02,       // bInterfaceSubClass (ACM subclass)
    0x01,       // bInterfaceProtocol (AT modem protocol)
    0x00,       // iInterface (none)
    0x05,       // bFunctionLength (CDC header descriptor starts here)
    0x24,       // bDescriptorType
    0x00,       // bDescriptorSubtype (CDC header descirptor)
    0x10,       // bcdCDC (low byte)
    0x01,       // bcdCDC (high byte)
    0x04,       // bFunctionLength (CDC abstract control management descirptor starts here)
    0x24,       // bDescriptorType
    0x02,       // bDescriptorSubtype (CDC abstract control management descriptor)
    0x02,       // bmCapabilities
    0x05,       // bFunctionLength (CDC union desciptor starts here)
    0x24,       // bDescriptorType
    0x06,       // bDescriptorSubtype (CDC union descriptor)
    0x00,       // bControlInterface
    0x01,       // bSubordinateInterface0
    0x05,       // bFunctionLength (Call management descriptor starts here)
    0x24,       // bDescriptorType
    0x01,       // bDescriptorSubtype (Call management descriptor)
    0x01,       // bmCapabilities
    0x01,       // bInterfaceNum
    0x07,       // bLength (Endpoint1 IN descriptor starts here)
    ENDPOINT,   // bDescriptorType
    0x81,       // bEndpointAddress (EP1 IN)
    0x03,       // bmAttributes (Interrupt)
    0x0A,       // wMaxPacketSize (low byte)
    0x00,       // wMaxPacketSize (high byte)
    0x40,       // bInterval
    0x09,       // bLength (Interface1 descriptor starts here)
    INTERFACE,  // bDescriptorType
    0x01,       // bInterfaceNumber
    0x00,       // bAlternateSetting
    0x02,       // bNumEndpoints
    0x0A,       // bInterfaceClass
    0x00,       // bInterfaceSubclass
    0x00,       // bInterfaceProtocol (No protocol)
    0x00,       // iInterface
    0x07,       // bLength (Endpoint2 OUT descriptor starts here)
    ENDPOINT,   // bDescriptorType
    0x02,       // bEndpointAddress (EP2 OUT)
    0x02,       // bmAttributes (Bulk)
    0x40,       // wMaxPacketSize (low byte)
    0x00,       // wMaxPacketSize (high byte)
    0x00,       // bInterval
    0x07,       // bLength (Endpoint2 IN descriptor starts here)
    ENDPOINT,   // bDescriptorType
    0x82,       // bEndpointAddress (EP2 IN)
    0x02,       // bmAttributes (Bulk)
    0x40,       // wMaxPacketSize (low byte)
    0x00,       // wMaxPacketSize (high byte)
    0x00        // bInterval
};

uint8_t __attribute__ ((space(auto_psv))) *Configurations[] = {
    Configuration1
};

uint8_t __attribute__ ((space(auto_psv))) String0[] = {
    4,          // bLength
    STRING,     // bDescriptorType
    0x09,       // wLANGID[0] (low byte)
    0x04        // wLANGID[0] (high byte)
};

uint8_t __attribute__ ((space(auto_psv))) String1[] = {
    26,         // bLength
    STRING,     // bDescriptorType
    'O', 0, 'l', 0, 'i', 0, 'n', 0, ' ', 0, 
    'C', 0, 'o', 0, 'l', 0, 'l', 0, 'e', 0, 'g', 0, 'e', 0
};

uint8_t __attribute__ ((space(auto_psv))) String2[] = {
    54,         // bLength
    STRING,     // bDescriptorType
    'P', 0, 'I', 0, 'C', 0, 
    '2', 0, '4', 0, 'F', 0, 'J', 0, 
    '1', 0, '2', 0, '8', 0, 'G', 0, 'B', 0, '2', 0, '0', 0, '6', 0, ' ', 0, 
    'C', 0, 'D', 0, 'C', 0, ' ', 0, 
    'D', 0, 'e', 0, 'v', 0, 'i', 0, 'c', 0, 'e', 0
};

uint8_t __attribute__ ((space(auto_psv))) *Strings[] = {
    String0, 
    String1, 
    String2
};

