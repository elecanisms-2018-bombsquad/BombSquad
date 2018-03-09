#ifndef _USB_H_
#define _USB_H_
#define	NUM_CONFIGURATIONS	1
#define	NUM_INTERFACES		1
#define NUM_STRINGS			2
#define MAX_PACKET_SIZE		64	// maximum packet size for low-speed peripherals is 8 bytes, for full-speed peripherals it can be 8, 16, 32, or 64 bytes

// states that the USB interface can be in
#define	POWERED_STATE	0x00
#define	DEFAULT_STATE	0x01
#define	ADDRESS_STATE	0x02
#define	CONFIG_STATE	0x03

// values for EP control registers
#define	ENDPT_DISABLED		0x00
#define ENDPT_IN_ONLY		0x05
#define ENDPT_OUT_ONLY		0x09
#define ENDPT_CONTROL		0x0D	// enable for in, out and setup
#define ENDPT_NON_CONTROL	0x1D	// enable for in, and out

// values for the PID bits in the BD status register
#define TOKEN_OUT	(0x01<<2)
#define TOKEN_ACK	(0x02<<2)
#define TOKEN_IN	(0x09<<2)
#define TOKEN_SETUP	(0x0D<<2)

// standard descriptor types
#define	DEVICE		1
#define	CONFIGURATION	2
#define	STRING		3
#define	INTERFACE	4
#define	ENDPOINT	5

// HID class descriptor types
#define HID			0x21
#define REPORT		0x22
#define PHYSICAL	0x23

// Standard USB requests
#define NO_REQUEST		0xFF
#define	GET_STATUS		0x00
#define	CLEAR_FEATURE	0x01
#define	SET_FEATURE		0x03
#define	SET_ADDRESS		0x05
#define	GET_DESCRIPTOR	0x06
#define SET_DESCRIPTOR	0x07
#define	GET_CONFIGURATION	0x08
#define	SET_CONFIGURATION	0x09
#define	GET_INTERFACE	0x0A
#define	SET_INTERFACE	0x0B
#define SYNCH_FRAME		0x0C

// HID Class requests
#define GET_REPORT		0x01
#define GET_IDLE		0x02
#define GET_PROTOCOL	0x03
#define SET_REPORT		0x09
#define SET_IDLE		0x0A
#define SET_PROTOCOL	0x0B

#define EP0				0x00
#define EP1				0x10
#define EP2				0x20
#define EP3				0x30
#define EP4				0x40
#define EP5				0x50
#define EP6				0x60
#define EP7				0x70
#define EP8				0x80
#define EP9				0x90
#define EP10			0xA0
#define EP11			0xB0
#define EP12			0xC0
#define EP13			0xD0
#define EP14			0xE0
#define EP15			0xF0

#define STANDARD_REQ	0x00 << 5
#define CLASS_REQ		0x01 << 5
#define VENDOR_REQ		0x02 << 5

#define RECIPIENT_DEVICE	0x00
#define RECIPIENT_INTERFACE	0x01
#define RECIPIENT_ENDPOINT	0x02

#define DEVICE_REMOTE_WAKEUP	0x01
#define ENDPOINT_HALT			0x00

// Bit masks to clear flags in the USB OTG Interrupt Status Register (host mode)
#define U1OTGIR_IDIF            0x80
#define U1OTGIR_T1MSECIF        0x40
#define U1OTGIR_LSTATEIF        0x20
#define U1OTGIR_ACTVIF          0x10
#define U1OTGIR_SESVDIF         0x08
#define U1OTGIR_SESENDIF        0x04
#define U1OTGIR_VBUSVDIF        0x01

// Bit masks to clear flags in the USB Interrupt Status Register
#define U1IR_STALLIF            0x80
#define U1IR_ATTACHIF           0x40    // pertaints only to host mode
#define U1IR_RESUMEIF           0x20
#define U1IR_IDLEIF             0x10
#define U1IR_TRNIF              0x08
#define U1IR_SOFIF              0x04
#define U1IR_UERRIF             0x02
#define U1IR_URSTIF             0x01

// Bit masks to clear flags in the USB Error Interrupt Status Register
#define U1EIR_BTSEF             0x80
#define U1EIR_DMAEF             0x20
#define U1EIR_BTOEF             0x10
#define U1EIR_DFN8EF            0x08
#define U1EIR_CRC16EF           0x04
#define U1EIR_CRC5EF            0x02    // pertains only to device mode
#define U1EIR_EOFEF             0x02    // pertains only to host mode
#define U1EIR_PIDEF             0x01

// Offsets into the buffer descriptor table
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

typedef unsigned char BYTE;

typedef union {
	int i;
	unsigned int w;
	BYTE b[2];
} WORD;

typedef union {
	long l;
	unsigned long ul;
	unsigned int w[2];
	BYTE b[4];
} WORD32;

typedef struct {
	BYTE __attribute__ ((packed)) bytecount;
	BYTE __attribute__ ((packed)) status;
	BYTE *address;
} BUFDESC;

typedef struct {
	BYTE __attribute__ ((packed)) bmRequestType;
	BYTE __attribute__ ((packed)) bRequest;
	WORD wValue;
	WORD wIndex;
	WORD wLength;
} SETUP;

typedef struct {
	SETUP setup;
	BYTE *data_ptr;
	WORD bytes_left;
} REQUEST;

extern __attribute__ ((aligned (512))) BUFDESC BD[];

extern BYTE EP0_OUT_buffer[];
extern BYTE EP0_IN_buffer[];

extern BUFDESC USB_buffer_desc;
extern SETUP USB_setup;
extern REQUEST USB_request;
extern BYTE USB_error_flags;
extern BYTE USB_curr_config;
extern BYTE USB_device_status;
extern BYTE USB_USTAT;
extern BYTE USB_USWSTAT;

extern BYTE __attribute__ ((space(auto_psv))) Device[];
extern BYTE __attribute__ ((space(auto_psv))) Configuration1[];
extern BYTE __attribute__ ((space(auto_psv))) String0[];
extern BYTE __attribute__ ((space(auto_psv))) String1[];
extern BYTE __attribute__ ((space(auto_psv))) String2[];

extern void ProcessSetupToken(void);
extern void ProcessInToken(void);
extern void ProcessOutToken(void);
extern void StandardRequests(void);
extern void SendDataPacket(void);
//extern void ClassRequests(void);
extern void VendorRequests(void);
extern void VendorRequestsIn(void);
extern void VendorRequestsOut(void);

#endif
