#include <p24FJ128GB206.h>
#include "elecanisms.h"
#include <stdio.h>
#include "usb.h"

#define TOGGLE_LED1         0
#define TOGGLE_LED2         1
#define TOGGLE_LED3         2
#define READ_SW1            3
#define READ_SW2            4
#define READ_SW3            5
#define READ_ENCODER        6

#define ENC_MOSI  D0
#define ENC_MISO  D1
#define ENC_SCK   D2
#define ENC_NCS   D3
#define ENC_MOSI_DIR  D0_DIR
#define ENC_MISO_DIR  D1_DIR
#define ENC_SCK_DIR   D2_DIR
#define ENC_NCS_DIR   D3_DIR

WORD enc_readReg(WORD address) {
    WORD cmd, result;
    cmd.w = 0x4000|address.w; //set 2nd MSB to 1 for a read
    cmd.w |= parity(cmd.w)<<15; //calculate even parity for

    ENC_NCS = 0; //lower the chip select line to start transfer
    SPI1BUF = (uint16_t)cmd.w;
    while (SPI1STATbits.SPIRBF ==0) {}
    result.w = SPI1BUF;
    ENC_NCS = 1;

    ENC_NCS = 0;
    SPI1BUF = 0;
    while (SPI1STATbits.SPIRBF ==0) {}
    result.w = SPI1BUF;
    ENC_NCS=1;

    return result;
}

void vendor_requests(void) {
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
        case READ_ENCODER:
            temp = enc_readReg(USB_setup.wValue);
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];
            BD[EP0IN].bytecount = 2;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        default:
            USB_error_flags |= REQUEST_ERROR;
    }
}



int16_t main(void) {
  uint8_t *RPOR, *RPINR;

  init_elecanisms();

  ENC_MOSI_DIR = OUT;
  ENC_MISO_DIR = IN;
  ENC_SCK_DIR = OUT;
  ENC_NCS_DIR = OUT;

  ENC_NCS = 1; //Raise the chip select line (it's active low). 

  RPOR = (uint8_t *)&RPOR0;
  RPINR = (uint8_t *)&RPINR0;

  __builtin_write_OSCCONL(OSCCON & 0xBF);
  RPOR[D0_RP] = MOSI1_RP;
  RPINR[MISO1_RP] = D1_RP;
  RPOR[D2_RP] = SCK1OUT_RP;
  __builtin_write_OSCCONL(OSCCON | 0x40);

  SPI1CON1 = 0x0132;              // SPI mode = 0, SCK freq = 1 MHz
  SPI1CON2 = 0;
  SPI1STAT = 0x8000;

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

