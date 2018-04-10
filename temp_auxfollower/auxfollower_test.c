#include <p24FJ128GB206.h>
#include <stdint.h>
#include "elecanisms.h"
#include "i2c_reg.h"
// #include "followertest.h"



#define s1248       0   // 539
#define s124        1   // 556
#define s128        2   // 576
#define s12         3   // 595
#define s148        4   // 618
#define s14         5   // 639
#define s18         6   // 664
#define s1          7   // 688      //these tw0
#define s248        8   // 695      // are closest
#define s24         9   // 721
#define s28         10  // 748
#define s2          11  // 775
#define s48         12  // 803
#define s4          13  // 830
#define s8          14  // 856
#define s           15  // 881
#define soops       16  // other
#define szero       17  // other
#define sbig        18  // other

uint16_t state;

int a = 169 ;
int b = 221 ;
int c = 275 ;
int d = 327 ;
int e = 383 ;
int f = 434 ;
int g = 488 ;
int h = 540 ;
int i = 555 ;
int j = 606 ;
int k = 657 ;
int l = 757 ;
int m = 802 ;
int n = 845 ;
int o = 883 ;

int ab, bc, cd, de, ef, fg, gh, hi, ij, jk, kl, lm, mn, no;

//////////////////////

#define SLAVE_ADDR 0x60

uint16_t waitwritedata;
uint16_t sendreaddata;
uint16_t datasend = 0x05;

typedef enum  {
  STATE_WAIT_FOR_ADDR,
  STATE_WAIT_FOR_WRITE_DATA,
  STATE_SEND_READ_DATA,
  STATE_SEND_READ_LAST
} STATE;

volatile STATE e_mystate = STATE_WAIT_FOR_ADDR;

void ledoff(void){ LED1 = 0; LED2 = 0; LED3 = 0; }

void __attribute__((interrupt, auto_psv)) _SI2C3Interrupt(void) {
    uint8_t u8_c;
    _SI2C3IF = 0;

    switch (e_mystate) {

    case STATE_WAIT_FOR_ADDR:
        u8_c = I2C3RCV;     //clear RBF bit for address
        if (I2C3STATbits.R_W) {     // if data transfer is out from slave // check R/W bit of address byte for read or write transaction
            I2C3TRN = datasend;   //Set transmit register to first data byte to send back to master
            I2C3CONbits.SCLREL = 1;     //release clock line so MASTER can drive it// SCL released during slave read
            e_mystate = STATE_SEND_READ_LAST; //goes to read transaction
        }
        else e_mystate = STATE_WAIT_FOR_WRITE_DATA;     // goes to write transaction
        break;

    case STATE_WAIT_FOR_WRITE_DATA:     // write transaction state    //character arrived, place in buffer
        waitwritedata = I2C3RCV;        //read the byte
        e_mystate = STATE_WAIT_FOR_ADDR;    // go to wait for next transaction
        break;

    case STATE_SEND_READ_LAST:  //interrupt for last character finished shifting out
        e_mystate = STATE_WAIT_FOR_ADDR;
        break;

    default:
        e_mystate = STATE_WAIT_FOR_ADDR;

    } // end of switch statements
}   // end of ISR

int16_t main(void) {

    ab = (b-a) >>1 ;
    bc = (c-b) >>1  ;
    cd = (d-c) >>1  ;
    de = (e-d) >>1  ;
    ef = (f-e) >>1  ;
    fg = (g-f) >>1  ;
    gh = (h-g) >>1  ;
    hi = (i-h) >>1  ;
    ij = (j-i) >>1  ;
    jk = (k-j) >>1  ;
    kl = (l-k) >>1  ;
    lm = (m-l) >>1  ;
    mn = (n-m) >>1  ;
    no = (o-n) >>1  ;


    init_elecanisms();
    i2c_init(157);      // Initializes I2C on I2C3

    I2C3ADD = SLAVE_ADDR>>1;   //initialize the address register
    I2C3MSK = 0;

    // _SI2C1IP = 1;        // Maybe eventially fo this for interrupt priority

    _SI2C3IE = 1;       // these two are the same! The underscore is cool.

    while (1) {
        uint16_t val = read_analog(A0_AN);

        if( val < (a+ab) ) {state = s;}                       //b
        if( val > (b-ab) && val < (b+bc) ) {state = s8;}      //b
        if( val > (c-bc) && val < (c+cd) ) {state = s4;}      //c
        if( val > (d-cd) && val < (d+de) ) {state = s48;}     //d
        if( val > (e-de) && val < (e+ef) ) {state = s2;}      //e
        if( val > (f-ef) && val < (f+fg) ) {state = s28;}     //f
        if( val > (g-fg) && val < (g+gh) ) {state = s24;}     //g
        if( val > (h-gh) && val < (h+hi) ) {state = s248;}    //h
        if( val > (i-hi) && val < (i+ij) ) {state = s1;}      //i
        if( val > (j-ij) && val < (j+jk) ) {state = s18;}     //j
        if( val > (k-jk) && val < (k+kl) ) {state = s14;}     //k
        if( val > (l-kl) && val < (l+lm) ) {state = s12;}     //l
        if( val > (m-lm) && val < (m+mn) ) {state = s128;}    //m
        if( val > (n-mn) && val < (n+no) ) {state = s124;}    //n
        if( val > (o-no) ) {state = s1248;}                 //o

        switch(state){

            case s1:    ledoff(); LED1 = 1; datasend = 0x01; break;
            case s2:    ledoff(); LED1 = 1; datasend = 0x02; break;
            case s4:    ledoff(); LED1 = 1; datasend = 0x03; break;
            case s8:    ledoff(); LED1 = 1; datasend = 0x04; break;

            case s12:   ledoff(); LED2 = 1; datasend = 0x05; break;
            case s14:   ledoff(); LED2 = 1; datasend = 0x06; break;
            case s18:   ledoff(); LED2 = 1; datasend = 0x07; break;
            case s24:   ledoff(); LED2 = 1; datasend = 0x08; break;
            case s28:   ledoff(); LED2 = 1; datasend = 0x09; break;
            case s48:   ledoff(); LED2 = 1; datasend = 0x10; break;

            case s124:  ledoff(); LED3 = 1; datasend = 0x11; break;
            case s128:  ledoff(); LED3 = 1; datasend = 0x12; break;
            case s148:  ledoff(); LED3 = 1; datasend = 0x13; break;
            case s248:  ledoff(); LED3 = 1; datasend = 0x14; break;
            case s1248: ledoff(); LED3 = 1; datasend = 0x15; break;

            case s:     ledoff(); datasend = 0x16; break;
            case soops: ledoff(); datasend = 0x17; break;
            case sbig:  ledoff(); datasend = 0x18; break;

            // default:
            //     LED1 = 1;




        } // end of switch statement

        // delay_by_nop(30000);
        // LED3 = !LED3;
        // delay_by_nop(30000);
    }

}






//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////





/*

#include <p24FJ128GB206.h>
#include <stdint.h>
// #include <stdio.h>
#include "elecanisms.h"
#include "i2c_reg.h"
// #include "followertest.h"


#define s1248       0   // 539
#define s124        1   // 556
#define s128        2   // 576
#define s12         3   // 595
#define s148        4   // 618
#define s14         5   // 639
#define s18         6   // 664
#define s1          7   // 688      //these tw0
#define s248        8   // 695      // are closest
#define s24         9   // 721
#define s28         10  // 748
#define s2          11  // 775
#define s48         12  // 803
#define s4          13  // 830
#define s8          14  // 856
#define s           15  // 881
#define soops       16  // other
#define szero       17  // other
#define sbig        18  // other

uint16_t state;

#define SLAVE_ADDR 0x08

uint16_t waitwritedata;
uint16_t sendreaddata;
volatile uint16_t datasend;

typedef enum  {
  STATE_WAIT_FOR_ADDR,
  STATE_WAIT_FOR_WRITE_DATA,
  STATE_SEND_READ_DATA,
  STATE_SEND_READ_LAST
} STATE;

volatile STATE e_mystate = STATE_WAIT_FOR_ADDR;

void ledoff(void){LED1 = 0;LED2 = 0;LED3 = 0;}

void __attribute__((interrupt, auto_psv)) _SI2C3Interrupt(void) {
    uint8_t u8_c;
    _SI2C3IF = 0;

    switch (e_mystate) {

    case STATE_WAIT_FOR_ADDR:
        u8_c = I2C3RCV;     //clear RBF bit for address
        if (I2C3STATbits.R_W) {     // if data transfer is out from slave // check R/W bit of address byte for read or write transaction
            I2C3TRN = datasend;   //Set transmit register to first data byte to send back to master
            I2C3CONbits.SCLREL = 1;     //release clock line so MASTER can drive it// SCL released during slave read
            e_mystate = STATE_SEND_READ_LAST; //goes to read transaction
        }
        else e_mystate = STATE_WAIT_FOR_WRITE_DATA;     // goes to write transaction
        break;

    case STATE_WAIT_FOR_WRITE_DATA:     // write transaction state    //character arrived, place in buffer
        waitwritedata = I2C3RCV;        //read the byte
        e_mystate = STATE_WAIT_FOR_ADDR;    // go to wait for next transaction
        break;

    case STATE_SEND_READ_LAST:  //interrupt for last character finished shifting out
        e_mystate = STATE_WAIT_FOR_ADDR;
        break;

    default:
        e_mystate = STATE_WAIT_FOR_ADDR;

    } // end of switch statements
}   // end of ISR

int16_t main(void) {
    init_elecanisms();

    i2c_init(157);      // Initializes I2C on I2C3
    I2C3ADD = SLAVE_ADDR>>1;   //initialize the address register
    I2C3MSK = 0;
    _SI2C3IE = 1;       // these two are the same! The underscore is cool.

    while(1){
        uint16_t val = read_analog(A0_AN);
        // if(val ==0 )              {state = szero;   }
        if(val > 500 && val < 548){state = s1248;   }
        if(val > 548 && val < 566){state = s124;    }
        if(val > 566 && val < 608){state = s128;    }
        if(val > 608 && val < 629){state = s12;     }
        if(val > 629 && val < 650){state = s148;    }
        if(val > 650 && val < 675){state = s14;     }
        if(val > 675 && val < 691){state = s18;     }
        if(val > 691 && val < 708){state = s1;      }
        if(val > 708 && val < 732){state = s248;    }
        if(val > 732 && val < 765){state = s24;     }
        if(val > 765 && val < 790){state = s28;     }
        if(val > 790 && val < 816){state = s2;      }
        if(val > 816 && val < 842){state = s48;     }
        if(val > 842 && val < 864){state = s4;      }
        if(val > 864 && val < 900){state = s8;      }
        if(val > 900 && val < 920){state = s;       }
        if(val < 500 )            {state = soops;   }
        if(val > 920 )            {state = sbig;   }

        switch(state){
            case soops:
                ledoff();
                LED3 = 1;
                datasend = 3;
                delay_by_nop(50);
            break;

            case sbig:
                ledoff();
                LED3 = 1;
                datasend = 3;
                delay_by_nop(50);
            break;

            case s1248:
                ledoff();
                LED2 = 1;
                datasend = 5;
                delay_by_nop(50);
            break;

            case s124:
                ledoff();
                LED1 = 1;
                datasend = 7;
                delay_by_nop(50);
            break;

        } // end of switch
    } //end of while loop

}


*/
