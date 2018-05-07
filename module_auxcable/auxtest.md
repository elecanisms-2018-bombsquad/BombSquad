#include "elecanisms.h"
#include "peripheral_core.h"
#include "i2c_address_space.h"
#include "ajuart.h"
#include "adafruit_led.h"

#define MODULE_LED_RED      D0
#define MODULE_LED_GREEN    D1

typedef void (*STATE_HANDLER_T)(void); // frame of game states
void setup(void);// forward declaration of module modes
void run(void);
void solved(void);
void end_win(void);
void end_fail(void);
STATE_HANDLER_T state, last_state;

typedef void (*GAME_STATE)(void);  // the sattes the games goes through
void first_gamestate(void);
void second_gamestate(void);
void third_gamestate(void);
GAME_STATE num_gamestate;

uint8_t done_striked;

void ledoff(void); // define function
// void updateReading(uint8_t oldarray, uint8_t targetarray );
void updateReading(uint8_t *oldarray, uint8_t *targetarray ){
    uint8_t i;
    for (i = 0; i < 6; i++){
        targetarray[i] = oldarray[i];
    }
}

void updateAnalog(uint8_t *array){
    uint8_t i;
    for (i = 0; i < 6; i++){
        array[i] = read_analog(A0_AN);
    }
}

// void test(uint8_t *a, uint8_t b){
//     // *a = *b; // put B into A
//     *a[0] = b[5];
// }

uint8_t prev_debounce_reading[2];
uint8_t new_debounce_reading[2];
uint8_t eval_reading[2];


uint8_t* valpointer;
uint8_t* valpointertwo;
uint8_t valarray[2];


void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
    // LED1 = 1;
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag

    valarray[0] = read_analog(A0_AN);
    valarray[1] = read_analog(A1_AN);
    valarray[2] = read_analog(A2_AN);

    valpointer[0] = valarray[0];
    valpointertwo = valarray;




    // updateAnalog(uint8_t prev_debounce_reading);
    // prev_debounce_reading = new_debounce_reading;
    // new_debounce_reading[0] = read_analog(A0_AN);
    // new_debounce_reading[1] = read_analog(A1_AN);
    // new_debounce_reading[2] = read_analog(A2_AN);
    // new_debounce_reading[3] = read_analog(A3_AN);
    // new_debounce_reading[4] = read_analog(A4_AN);
    // new_debounce_reading[5] = read_analog(A5_AN);
    // if (new_debounce_reading == prev_debounce_reading) {
    //     eval_reading = new_debounce_reading;
    // }

}



int16_t main(void) {
    init_elecanisms();
    init_ajuart();

    D0_DIR = OUT;
    D1_DIR = OUT;

    i2c2_init(157);                      // Initializes I2C on I2C2
    I2C2ADD = TEST_PERIPHERAL_ADDR>>1;   // Set the device address (7-bit register)
    I2C2MSK = 0;                         // Set mask to 0 (only this address matters)
    _SI2C2IE = 1;                        // Enable i2c slave interrupt

    T1CON = 0x0020;         // set Timer1cd .. period to 10 ms for debounce
    PR1 = 0x2710;           // prescaler 16, match value 10000
    TMR1 = 0;               // set Timer1 count to 0
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    IEC0bits.T1IE = 1;      // enable Timer1 interrupt
    T1CONbits.TON = 1;      // turn on Timer1

    state = setup;



    while (1) {
        state();
    }
} // end of main


// STATE MACHINE FUNCTIONS /////////////////////////////////////////////////////


void setup(void) { // Waits for master module to start the game
    // State Setup
    if (state != last_state) {
        last_state = state;
        MODULE_LED_GREEN = ON; delay_by_nop(1);
        MODULE_LED_RED = ON;
        complete_flag = 0;
        num_strikes = 0;
        error_code = 0;
        // setup state here
    }

    // Perform state tasks

    //Check for state transitions
    if ((start_flag == 1) || (SW2 == 0)){
        state = run;
    }

    // State Cleanup
    if (state != last_state) {
        MODULE_LED_RED = OFF; delay_by_nop(1);
        MODULE_LED_GREEN = OFF;
    }
}

void run(void) { // Plays the game
    // State Setup
    if (state != last_state) {
        last_state = state;
        LED1 = ON; delay_by_nop(1);
        MODULE_LED_RED = ON;
        done_striked = 0;
    }

    // Perform state tasks


    // Check for state transitions
    if (win_flag == 1) {
        state = end_win;
    } else if (lose_flag == 1) {
        state = end_fail;
    }
    if (SW2 == 0 ) {
        state = solved;
    }
    if (SW1 == 0) {
        if (!done_striked) {
            num_strikes++;
            done_striked = 1;
        }
    }
    if (SW3 == 0) {
        if (!done_striked) {
            num_strikes+=3;
            done_striked = 1;
        }
    }
    U1_putc(num_strikes);
    U1_putc('\r');
    U1_putc('\n');
    U1_flush_tx_buffer();

    // State Cleanup
    if (state != last_state) {
        LED1=OFF; delay_by_nop(1);
        MODULE_LED_RED = OFF;
    }
}

void solved(void) { // The puzzle on this module is finished
    // State Setup
    if (state != last_state) {
        last_state = state;
        LED3 = ON;
        complete_flag = 1;
        MODULE_LED_GREEN = ON;
    }

    // Perform state tasks


    // Check for state transitions
    if (win_flag == 1) {
        state = end_win;
    } else if (lose_flag == 1) {
        state = end_fail;
    }

    // State Cleanup
    if (state != last_state) {
        LED3 = OFF;
        complete_flag = 0;
        MODULE_LED_GREEN = OFF;
    }
}

void end_win(void) { // The master module said the game was won
    // State Setup
    if (state != last_state) {
        last_state = state;
        MODULE_LED_GREEN = ON;

        T1CON = 0x0030;         // set Timer1 period to 0.5s
        PR1 = 0x7A11;

        TMR1 = 0;               // set Timer1 count to 0
        IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
        T1CONbits.TON = 1;      // turn on Timer1
        // setup state here
    }

    // Perform state tasks
    if (IFS0bits.T1IF == 1) {
        IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
        MODULE_LED_GREEN = !MODULE_LED_GREEN;           // toggle LED
    }


    // State Cleanup
    if (state != last_state) {
        MODULE_LED_GREEN = OFF;
    }
}

void end_fail(void) { // The master module said the game was lost
    // State Setup
    if (state != last_state) {
        last_state = state;
        MODULE_LED_RED = ON;

        T1CON = 0x0030;         // set Timer1 period to 0.5s
        PR1 = 0x7A11;

        TMR1 = 0;               // set Timer1 count to 0
        IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
        T1CONbits.TON = 1;      // turn on Timer1
    }

    // Perform state tasks
    if (IFS0bits.T1IF == 1) {
        IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
        MODULE_LED_RED = !MODULE_LED_RED;           // toggle LED
    }


    // State Cleanup
    if (state != last_state) {
        MODULE_LED_RED = OFF;
        T1CONbits.TON = 0;      // turn off Timer1
    }
}

void ledoff(void) {
    LED1 = 0; delay_by_nop(1);
    LED2 = 0; delay_by_nop(1);
    LED3 = 0; delay_by_nop(1);
    D0 = OFF;
}












/*
// Old stuff from last time

#include <p24FJ128GB206.h>
#include <stdint.h>
#include <stdio.h>
#include "elecanisms.h"
#include "i2c_reg.h"

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


void ledoff(void){ LED1 = 0; LED2 = 0; LED3 = 0; }

void delay_by_nop(uint32_t num_nops){    // 1 nop= 375ns
    uint32_t count = 0;
    while (count < num_nops){
        __asm__("nop");
        count +=1;
    }
}

uint16_t state;

int16_t main(void) {
    init_elecanisms();



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
            // delay_by_nop(30000);
            LED3 = 1;
            // delay_by_nop(30000);
        break;

        case sbig:
            ledoff();
            // delay_by_nop(30000);
            LED3 = 1;
            // delay_by_nop(30000);
        break;

        case s1248:
            ledoff();
            // delay_by_nop(30000);
            LED2 = 1;
            // delay_by_nop(30000);
        break;

        case s124:
            ledoff();
            // delay_by_nop(30000);
            LED1 = 1;
            // delay_by_nop(30000);
        break;

        // default:
        //     state = soops;

    } // end of switch
} //end of while loop


} // end of main

*/
