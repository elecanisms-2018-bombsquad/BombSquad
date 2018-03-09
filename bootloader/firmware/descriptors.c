#include <p24FJ128GB206.h>
#include "usb.h"

BYTE __attribute__ ((space(auto_psv))) Device[] = {
    0x12,       // bLength
    DEVICE,     // bDescriptorType
    0x00,       // bcdUSB (low byte)
    0x02,       // bcdUSB (high byte)
    0x00,       // bDeviceClass
    0x00,       // bDeviceSubClass
    0x00,       // bDeviceProtocol
    MAX_PACKET_SIZE,    // bMaxPacketSize
    0x66,       // idVendor (low byte)
    0x66,       // idVendor (high byte)
    0x21,       // idProduct (low byte)
    0x43,       // idProduct (high byte)
    0x00,       // bcdDevice (low byte)
    0x00,       // bcdDevice (high byte)
    0x01,       // iManufacturer
    0x02,       // iProduct
    0x00,       // iSerialNumber (none)
    NUM_CONFIGURATIONS    // bNumConfigurations
};

BYTE __attribute__ ((space(auto_psv))) Configuration1[] = {
    0x09,       // bLength
    CONFIGURATION,    // bDescriptorType
    0x12,       // wTotalLength (low byte)
    0x00,       // wTotalLength (high byte)
    NUM_INTERFACES,   // bNumInterfaces
    0x01,       // bConfigurationValue
    0x00,       // iConfiguration (none)
    0xA0,       // bmAttributes
    0x32,       // bMaxPower (100 mA)
    0x09,       // bLength (Interface1 descriptor starts here)
    INTERFACE,  // bDescriptorType
    0x00,       // bInterfaceNumber
    0x00,       // bAlternateSetting
    0x00,       // bNumEndpoints (excluding EP0)
    0xFF,       // bInterfaceClass (vendor specific class code)
    0x00,       // bInterfaceSubClass
    0xFF,       // bInterfaceProtocol (vendor specific protocol used)
    0x00        // iInterface (none)
};

BYTE __attribute__ ((space(auto_psv))) String0[] = {
    0x04,       // bLength
    STRING,     // bDescriptorType
    0x09,       // wLANGID[0] (low byte)
    0x04        // wLANGID[0] (high byte)
};

BYTE __attribute__ ((space(auto_psv))) String1[] = {
    0x1A,       // bLength
    STRING,     // bDescriptorType
    'O', 0x00, 'l', 0x00, 'i', 0x00, 'n', 0x00, ' ', 0x00,
    'C', 0x00, 'o', 0x00, 'l', 0x00, 'l', 0x00, 'e', 0x00, 'g', 0x00, 'e', 0x00
};

BYTE __attribute__ ((space(auto_psv))) String2[] = {
    46,         // bLength
    STRING,     // bDescriptorType
    'P', 0x00, 'I', 0x00, 'C', 0x00, '2', 0x00, '4', 0x00, 'F', 0x00, 'J', 0x00, ' ', 0x00, 'U', 0x00, 'S', 0x00, 'B', 0x00, ' ', 0x00,
    'B', 0x00, 'o', 0x00, 'o', 0x00, 't', 0x00, 'l', 0x00, 'o', 0x00, 'a', 0x00, 'd', 0x00, 'e', 0x00, 'r', 0x00
};
