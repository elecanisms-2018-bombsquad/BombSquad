/*
** FSM code built on basis of fsm0 script.
*/
#include "elecanisms.h"
#include "lcd.h"


typedef void (*STATE_HANDLER_T)(void);

void idle(void);
void active(void);
void end(void);

STATE_HANDLER_T state, last_state;
uint16_t time_left;

_LCD lcd_screen;

void idle(void) {
    if (state != last_state) {  // if we are entering the state, do initialization stuff
        last_state = state;
        LED1 = ON;
        D3 = ON;
        lcd_print2(&lcd_screen, "BombSquad", "Ready");
    }

    // Perform state tasks

    // Check for state transitions
    if (D2 == 0) {
        state = active;
    }

    if (state != last_state) {  // if we are leaving the state, do clean up stuff
        LED1 = OFF;
        D3 = OFF;
    }
}

void active(void) {
    if (state != last_state) {  // if we are entering the state, do intitialization stuff
        last_state = state;
        D3 = ON;
        IFS0bits.T1IF = 0;
        TMR1 = 0;
        T1CONbits.TON = 1;
        time_left = 20;
        lcd_print2(&lcd_screen, "Counting Down", "");
    }

    // Perform state tasks
    if (IFS0bits.T1IF == 1) {
        IFS0bits.T1IF = 0;
        D3 = !D3;
        char * ptr;
        // lcd_print2(&lcd_screen, itoa(time_left, ptr, 10), "");
        time_left--;
    }


    // Check for state transitions
    if (SW2 == 0) {
        state = idle;
    } else if (time_left == 0) {
        state = end;
    }

    if (state != last_state) {  // if we are leaving the state, do clean up stuff
        D3 = OFF;
        T1CONbits.TON = 0;
    }
}

void end(void) {
    if (state != last_state) {  // if we are entering the state, do initialization stuff
        last_state = state;
        LED3 = ON;
        lcd_print2(&lcd_screen, "You Lose", "Sad");
    }

    // Perform state tasks

    // Check for state transitions

    if (state != last_state) {  // if we are leaving the state, do clean up stuff
        LED3 = OFF;
    }
}

int16_t main(void) {
    init_elecanisms();
    i2c_init(1e3);
    lcd_init(&lcd_screen, 0x06, 'A');
    lcd_clear(&lcd_screen);

    // Set d3, d4 as output
    D3_DIR = 0; D4_DIR = 0;
    // Write button LED on
    D3 = 1; D4 = 0;

    // Write internal pull-up for D2 high
    CNPU2bits.CN17PUE = 1;

    T1CON = 0x0030;         // set Timer1 period to 0.5s
    PR1 = 0x7A11;

    TMR1 = 0;               // set Timer1 count to 0
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag

    state = idle;
    last_state = (STATE_HANDLER_T)NULL;

    while (1) {
        state();
    }
}
