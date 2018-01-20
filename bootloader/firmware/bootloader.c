#include <p24FJ128GB206.h>
#include "usb.h"

#define LED1        PORTDbits.RD7
#define LED2        PORTFbits.RF0
#define LED3        PORTFbits.RF1

#define LED1TRIS    TRISDbits.TRISD7
#define LED2TRIS    TRISFbits.TRISF0
#define LED3TRIS    TRISFbits.TRISF1

#define SW1         PORTCbits.RC15
#define SW2         PORTCbits.RC12
#define SW3         PORTBbits.RB12

#define SW1TRIS     TRISCbits.TRISC15
#define SW2TRIS     TRISCbits.TRISC12
#define SW3TRIS     TRISBbits.TRISB12

#define VBUS        PORTFbits.RF7
#define VBUSTRIS    TRISFbits.TRISF7

_CONFIG1(FWDTEN_OFF & WINDIS_OFF & ICS_PGx2 & GWRP_OFF & GCP_OFF & JTAGEN_OFF);
_CONFIG2(POSCMOD_NONE & IOL1WAY_OFF & OSCIOFNC_ON & FCKSM_CSECME & FNOSC_FRCPLL & PLL96MHZ_ON & PLLDIV_NODIV & IESO_ON);
_CONFIG3(SOSCSEL_EC & WUTSEL_LEG & WPDIS_WPEN & WPCFG_WPCFGEN & WPEND_WPSTARTMEM & WPFP_WPFP3);
//_CONFIG3(SOSCSEL_EC & WUTSEL_LEG & WPDIS_WPDIS & WPCFG_WPCFGDIS);

//#define    SHOW_ENUM_STATUS

#define READ_FLASH  2
#define WRITE_FLASH 3
#define ERASE_FLASH 4
#define START_USER  6

BYTE BOOT_QUIT;
WORD32 BOOT_COUNTDOWN;

//void ClassRequests(void) {
//    switch (USB_setup.bRequest) {
//        default:
//            USB_error_flags |= 0x01;                    // set Request Error Flag
//    }
//}

void VendorRequests(void) {
    unsigned int temp, i;
    WORD data;

    switch (USB_setup.bRequest) {
        case READ_FLASH:
            temp = TBLPAG;                 // save the value of TBLPAG
            TBLPAG = USB_setup.wValue.w;
            for (i = 0; i<USB_setup.wLength.w; USB_setup.wIndex.w += 2) {
                data.w = __builtin_tblrdl(USB_setup.wIndex.w);
                BD[EP0IN].address[i++] = data.b[0];
                BD[EP0IN].address[i++] = data.b[1];
                data.w = __builtin_tblrdh(USB_setup.wIndex.w);
                BD[EP0IN].address[i++] = data.b[0];
                BD[EP0IN].address[i++] = data.b[1];
            }
            TBLPAG = temp;                 // restore original value to TBLPAG
            BD[EP0IN].bytecount = (BYTE)USB_setup.wLength.w;
            BD[EP0IN].status = 0xC8;
            break;
        case WRITE_FLASH:
            USB_request.setup.bmRequestType = USB_setup.bmRequestType;  // processing a WRITE_FLASH request
            USB_request.setup.bRequest = USB_setup.bRequest;
            USB_request.setup.wValue.w = USB_setup.wValue.w;
            USB_request.setup.wIndex.w = USB_setup.wIndex.w;
            USB_request.setup.wLength.w = USB_setup.wLength.w;
            break;
        case ERASE_FLASH:
            NVMCON = 0x4042;                // set up NVMCON to erase a page of program memory
            temp = TBLPAG;                  // save the value of TBLPAG
            TBLPAG = USB_setup.wValue.w;
            __builtin_tblwtl(USB_setup.wIndex.w, 0x0000);
            __asm__("DISI #16");            // disable interrupts for 16 cycles
            __builtin_write_NVM();          // issue the unlock sequence and perform the write
            while (NVMCONbits.WR==1) {}     // wait until the write is complete
            NVMCONbits.WREN = 0;            // disable further writes to program memory
            TBLPAG = temp;                  // restore original value to TBLPAG
            BD[EP0IN].bytecount = 0x00;     // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;        // send packet as DATA1, set UOWN bit
            break;
        case START_USER:
            BOOT_QUIT = 1;
            BOOT_COUNTDOWN.w[0] = USB_setup.wValue.w;
            BOOT_COUNTDOWN.w[1] = USB_setup.wIndex.w;
            BD[EP0IN].bytecount = 0x00;     // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;        // send packet as DATA1, set UOWN bit
            break;
        default:
            USB_error_flags |= 0x01;        // set Request Error Flag
    }
}

void VendorRequestsIn(void) {
    switch (USB_request.setup.bRequest) {
        default:
            USB_error_flags |= 0x01;                    // set Request Error Flag
    }
}

void VendorRequestsOut(void) {
    unsigned int temp, row, i;
    WORD data;

    switch (USB_request.setup.bRequest) {
        case WRITE_FLASH:
            NVMCON = 0x4001;                // set up NVMCON to program a row of program memory
            temp = TBLPAG;                  // save the value of TBLPAG
            TBLPAG = USB_request.setup.wValue.w;
            row = USB_request.setup.wIndex.w&0xFFF8;
            for (i = 0; i<128; i += 2) {
                __builtin_tblwtl(row + i, 0xFFFF);
                __builtin_tblwth(row + i + 1, 0x00FF);
            }
            for (i = 0; i<USB_request.setup.wLength.w; USB_request.setup.wIndex.w += 2) {
                data.b[0] = BD[EP0OUT].address[i++];
                data.b[1] = BD[EP0OUT].address[i++];
                __builtin_tblwtl(USB_request.setup.wIndex.w, data.w);
                data.b[0] = BD[EP0OUT].address[i++];
                data.b[1] = BD[EP0OUT].address[i++];
                __builtin_tblwth(USB_request.setup.wIndex.w, data.w);
            }
            __asm__("DISI #16");            // disable interrupts for 16 cycles
            __builtin_write_NVM();          // issue the unlock sequence and perform the write
            while (NVMCONbits.WR==1) {}     // wait until the write is done
            NVMCONbits.WREN = 0;            // disable further writes to program memory
            TBLPAG = temp;                  // restore original value to TBLPAG
            break;
        default:
            USB_error_flags |= 0x01;                    // set Request Error Flag
    }
}

int main(void) {
    CLKDIV = 0x0100;    // RCDIV = 001 (4MHz, div2), CPDIV = 00 (FOSC = 32MHz, FCY = 16MHz)
    ANSB = 0x0000;      // Make all pins digital I/Os
    ANSC = 0x0000;
    ANSD = 0x0000;
    ANSF = 0x0000;
    ANSG = 0x0000;
    if (SW1) {
        __asm__("goto 0x1000");
    }
    T1CON = 0x8030;     // Configure and enable Timer1 (TCKPS<1:0> = 11, prescalar of 1:256)
    PR1 = 6250;         // Set Timer1 period register so Timer1 goes off in intervals of 100 ms
    LED1TRIS = 0;
    LED1 = 1;           // Turn on LED1
    while (!VBUS) {     // Blink LED1 at 5 Hz until USB is plugged in (i.e., VBUS is present)
        if (IFS0bits.T1IF) {    // If Timer1 has gone off...
            LED1 = !LED1;       // ...toggle LED1
            IFS0bits.T1IF = 0;  // ...and lower the Timer1 flag
        }
    }
    LED1 = 0;           // Turn off LED1
    InitUSB();          // Initialize the USB registers and serial interface engine
    BOOT_QUIT = 0;
    while (1) {
        if (BOOT_QUIT) {
            if (BOOT_COUNTDOWN.ul>0) {
                BOOT_COUNTDOWN.ul--;
            } else {
                __asm__("reset");
            }
        }
        ServiceUSB();   // Service any pending USB requests
    }
}
