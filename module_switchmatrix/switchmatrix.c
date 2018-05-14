#include <stdio.h>
#include "elecanisms.h"
#include "peripheral_core.h"
#include "i2c_address_space.h"
#include "ajuart.h"
#include "adafruit_led.h"

// picocom -b 115200 /dev/ttyUSB0

char buffer[128];

#define switch1  0b0000000001   // wait / okay
#define switch2  0b0000000010   // uh huh / uh uh
#define switch3  0b0000000100   // V / other V
#define switch4  0b0000001000   // blank / _
#define switch5  0b0000010000   // down / up
#define switch6  0b0000100000   // star / *
#define switch7  0b0001000000   // done / next
#define switch8  0b0010000000   // wait / no

uint8_t switchstring = 0b11100101;
uint8_t l1_activeswitches = 0b10001101; // the ones that are on l1
uint8_t l2_activeswitches = 0b01110010; // the ones that are on l2
uint8_t list1sum, list2sum;
uint8_t l1_missing, l2_missing;

uint16_t switches = 0;
uint16_t initial_switches = 0;
uint16_t switches_correct = 0;

#define MODULE_LED_RED      D0
#define MODULE_LED_GREEN    D1

typedef void (*STATE_HANDLER_T)(void); // frame of game states
void setup(void);// forward declaration of module modes
void run(void);
void solved(void);
void end_win(void);
void end_fail(void);
STATE_HANDLER_T state, last_state;

typedef void (*GAME_STATE)(void);  // the states the games goes through
void check_lists(void);
void listonebigger(void);
void listtwobigger(void);
void listsequal(void);
GAME_STATE gamestate;

uint8_t done_striked = 0;

void ledoff(void); // define function
void pinsetup(void);
void checkSwitches(void);
void checkInitialSwitches(void);


/////////////////////////////////////////////////////
/////////////// end of intial stuff /////////////////
/////////////////////////////////////////////////////

int16_t main(void) {
    init_elecanisms();
    init_ajuart();
    pinsetup();

    D0_DIR = OUT;
    D1_DIR = OUT;

    i2c2_init(157);                      // Initializes I2C on I2C2
    I2C2ADD = TEST_PERIPHERAL_ADDR>>1;   // Set the device address (7-bit register)
    I2C2MSK = 0;                         // Set mask to 0 (only this address matters)
    _SI2C2IE = 1;                        // Enable i2c slave interrupt

    state = setup;

    while (1) {
        state();

        // sprintf(buffer, "STRIKES %x ",
        // num_strikes);
        // U1_putc('\r'); U1_putc('\n');
        // U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
        // delay_by_nop(100);
    }
} // end of main


void listonebigger(){
    checkSwitches();
    uint8_t wingoal;
    uint8_t toprow = initial_switches & 0b00001111;
    uint8_t win1 = (toprow << 4);
    uint8_t win2 = (~toprow & 0b00001111);
    wingoal = (toprow << 4) | (~toprow & 0b00001111);

    // sprintf(buffer, "[ONE] IntialSwitches: %x || Switches:%x | toprow:%x | wingoal:%x || W1: %x | W2: %x ",
    //                 initial_switches, switches, toprow,  wingoal, win1, win2);
    // U1_putc('\r');
    // U1_putc('\n');
    // U1_puts((uint8_t*)buffer);
    // U1_flush_tx_buffer();
    // delay_by_nop(30000);

    if (switches == wingoal){switches_correct = 1;}
    else{switches_correct = 0;}


        // sprintf(buffer, "WIN One bigger");
        // U1_putc('\r');
        // U1_putc('\n');
        // U1_puts((uint8_t*)buffer);
        // U1_flush_tx_buffer();
        // delay_by_nop(300);
}


void listtwobigger(){
    checkSwitches();
    uint8_t wingoal;
    wingoal = (initial_switches & 0b01110010) | (0b10000101);

    // sprintf(buffer, "[TWO] IntialSwitches: %x || Switches:%x | wingoal:%x ",
    //                 initial_switches, switches, wingoal);
    // U1_putc('\r');
    // U1_putc('\n');
    // U1_puts((uint8_t*)buffer);
    // U1_flush_tx_buffer();
    // delay_by_nop(30000);
    //
    if (switches == wingoal){switches_correct = 1;}
    else{switches_correct = 0;}
    // sprintf(buffer, "WIN Two bigger");
    // U1_putc('\r');
    // U1_putc('\n');
    // U1_puts((uint8_t*)buffer);
    // U1_flush_tx_buffer();
    // delay_by_nop(300);
}





void listsequal(void) {
    checkSwitches();
    uint8_t binarygoal, wingoal;
    uint8_t trs = initial_switches & 0b00001111;        // toprow_switches
    uint8_t brs = (initial_switches >> 4) & 0b00001111; // botrow_switches

    uint8_t toprow_val = ((trs & 0b0001) << 3 ) + ((trs & 0b0010) << 1 ) + ((trs & 0b0100) >> 1 ) + ((trs & 0b1000) >> 3 );
    uint8_t botrow_val = ((brs & 0b0001) << 3 ) + ((brs & 0b0010) << 1 ) + ((brs & 0b0100) >> 1 ) + ((brs & 0b1000) >> 3 );

    if (toprow_val > botrow_val) { wingoal = (trs << 4) | trs; }
    else                         { wingoal = (brs << 4) | brs; }

    // sprintf(buffer, "[EQUAL] IntialSwitches: %x || Switches:%x | toprow:%x | botrow:%x | wingoal:%x ",
    //                 initial_switches, switches, toprow_val, botrow_val, wingoal);
    // U1_putc('\r');
    // U1_putc('\n');
    // U1_puts((uint8_t*)buffer);
    // U1_flush_tx_buffer();
    // delay_by_nop(30000);
    //
    if (switches == wingoal){switches_correct = 1;}
    else{switches_correct = 0;}
    // sprintf(buffer, "WIN lists equal");
    // U1_putc('\r');
    // U1_putc('\n');
    // U1_puts((uint8_t*)buffer);
    // U1_flush_tx_buffer();
    // delay_by_nop(300);
}



// STATE MACHINE FUNCTIONS /////////////////////////////////////////////////////

void setup(void) { // Waits for master module to start the game
    if (state != last_state) {
        last_state = state;
        MODULE_LED_GREEN = ON; delay_by_nop(1);
        MODULE_LED_RED = ON;
        complete_flag = 0;
        num_strikes = 0;
        start_flag = 0;
        error_code = 0;}

    gamestate = check_lists;
    checkInitialSwitches();
    done_striked = 0;

    if ((start_flag == 1) || (SW2 == 0)){
        state = run;
    }

    if (state != last_state) {
        MODULE_LED_RED = OFF; delay_by_nop(1);
        MODULE_LED_GREEN = OFF;}
}

void run(void) { // Plays the game
    if (state != last_state) {
        last_state = state;
        MODULE_LED_RED = ON;
    }
    gamestate();


    if (win_flag == 1) {state = end_win; }       // Check for state transitions
    else if (lose_flag == 1) {state = end_fail;}

    uint16_t reading = read_analog(A5_AN);
    if (reading > 500  ) {
        sprintf(buffer, "Reading: %x  ||  SwitchesCorrect: %x", reading, switches_correct);
        U1_putc('\r'); U1_putc('\n'); U1_puts((uint8_t*)buffer); U1_flush_tx_buffer(); delay_by_nop(10);

        if (switches_correct == 1) {
            state = solved;
            sprintf(buffer, "switchescorrect = 1 ", reading);
            U1_putc('\r'); U1_putc('\n'); U1_puts((uint8_t*)buffer); U1_flush_tx_buffer(); delay_by_nop(10);
        }

        else {
            if(done_striked == 0){
                num_strikes++;
                checkInitialSwitches();
                gamestate = check_lists;
                delay_by_nop(30);
                done_striked = 1;
            }
        }

        sprintf(buffer, "strikes: %x   ||  GameState ", num_strikes, gamestate );
        U1_putc('\r'); U1_putc('\n'); U1_puts((uint8_t*)buffer); U1_flush_tx_buffer(); delay_by_nop(10);
    }

    else {      // button is released
        done_striked = 0;
    }

    if (state != last_state) { MODULE_LED_RED = OFF;}     // State Cleanup


}

void solved(void) { // The puzzle on this module is finished
    if (state != last_state) {
        last_state = state;
        complete_flag = 1;
        MODULE_LED_GREEN = ON;}

    if (win_flag == 1) {state = end_win; }       // Check for state transitions
    else if (lose_flag == 1) {state = end_fail;}

    if (state != last_state) {      // State Cleanup
        complete_flag = 0;
        MODULE_LED_GREEN = OFF;}

        sprintf(buffer, "SOLVED");
        U1_putc('\r');
        U1_putc('\n');
        U1_puts((uint8_t*)buffer);
        U1_flush_tx_buffer();
        delay_by_nop(300000);
}

void end_win(void) { // The master module said the game was won
    if (state != last_state) {
        last_state = state;
        MODULE_LED_GREEN = ON;
        T1CON = 0x0030;         // set Timer1 period to 0.5s
        PR1 = 0x7A11;
        TMR1 = 0;               // set Timer1 count to 0
        IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
        T1CONbits.TON = 1;      // turn on Timer1
    }
    if (IFS0bits.T1IF == 1) {       // Perform state tasks
        IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
        MODULE_LED_GREEN = !MODULE_LED_GREEN;           // toggle LED
    }
    if (state != last_state) {  // State Cleanup
        MODULE_LED_GREEN = OFF;
    }
}

void end_fail(void) { // The master module said the game was lost
    if (state != last_state) {      // State Setup
        last_state = state;
        MODULE_LED_RED = ON;
        T1CON = 0x0030;         // set Timer1 period to 0.5s
        PR1 = 0x7A11;
        TMR1 = 0;               // set Timer1 count to 0
        IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
        T1CONbits.TON = 1;      // turn on Timer1
    }
    if (IFS0bits.T1IF == 1) {       // Perform state tasks
        IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
        MODULE_LED_RED = !MODULE_LED_RED;           // toggle LED
    }
    if (state != last_state) {      // State Cleanup
        MODULE_LED_RED = OFF;
        T1CONbits.TON = 0;      // turn off Timer1
    }
}

void check_lists(void){
    delay_by_nop(30000);
    checkSwitches();
    // figure out what is missing by xor-ing with the whole list of switch positions
    // then bitmask out the bits that correspond to the other list
    // this yields the bits that were wrong for that list
    l1_missing = (initial_switches ^ switchstring) & l1_activeswitches ;
    l2_missing = (initial_switches ^ switchstring) & l2_activeswitches ;

    list1sum =
        (  initial_switches & 0b00000001) +             // switch 1: on
        (( initial_switches & 0b00000100) >> 2 ) +      // switch 3: on
        (((~initial_switches) & 0b00001000) >> 3 ) +    // switch 4 : off
        (( initial_switches & 0b10000000) >> 7) ;       // switch 8 : on

    list2sum =
        (((~initial_switches) & 0b00000010) >> 1) +       // switch 2: off
        (((~initial_switches & 0b00010000)) >> 4 ) +      // switch 5: off
        ( ( initial_switches & 0b00100000)  >> 5 ) +      // switch 6: on
        ( ( initial_switches & 0b01000000)  >> 6) ;       // switch 7: on

    // sprintf(buffer, "INITIAL List1 %x |||  (1)%x  (3)%x  (4)%x  (8)%x ",
    // list1sum, initial_switches & 0b00000001 , (( initial_switches & 0b00000100) >> 2 ) , (((~initial_switches) & 0b00001000) >> 3 ) , (( initial_switches & 0b10000000) >> 7)
    // );
    // U1_putc('\r'); U1_putc('\n');
    // U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
    // delay_by_nop(100);
    //
    // sprintf(buffer, "INITIAL List2 %x |||  (2)%x  (5)%x  (6)%x  (7)%x ",
    // list2sum, (((~initial_switches) & 0b00000010) >> 1), (((~initial_switches & 0b00010000)) >> 4 ) , ( ( initial_switches & 0b00100000)  >> 5 ) , ( ( initial_switches & 0b01000000)  >> 6)
    // );
    // U1_putc('\r'); U1_putc('\n');
    // U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
    // delay_by_nop(100);
    //
    // sprintf(buffer, "Switches %x  ",
    // switches
    // );
    // U1_putc('\r'); U1_putc('\n');
    // U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
    // delay_by_nop(100);
    // //
    // sprintf(buffer, "-------------------------------------------------------------------");
    // U1_putc('\r'); U1_putc('\n');
    // U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
    // delay_by_nop(600000);


    if (list1sum > list2sum)  { gamestate = listonebigger; }
    if (list2sum > list1sum)  { gamestate = listtwobigger; }
    if (list1sum == list2sum) { gamestate = listsequal; }
    if (list1sum == 0 || list2sum == 0) {gamestate = listsequal;}

}

void ledoff(void) {
    LED1 = 0; delay_by_nop(1);
    LED2 = 0; delay_by_nop(1);
    LED3 = 0; delay_by_nop(1);
    D0 = OFF;
}

void pinsetup(void){
    D4_DIR = 1;  __asm__("nop");
    D5_DIR = 1;  __asm__("nop");
    D6_DIR = 1;  __asm__("nop");
    D7_DIR = 1;  __asm__("nop");
    D8_DIR = 1;  __asm__("nop");
    D9_DIR = 1;  __asm__("nop");
    D10_DIR = 1; __asm__("nop");
    D12_DIR = 1; __asm__("nop");
    D13_DIR = 1; __asm__("nop");

    D4_PUE = 1;  __asm__("nop");
    D5_PUE = 1;  __asm__("nop");
    D6_PUE = 1;  __asm__("nop");
    D7_PUE = 1;  __asm__("nop");
    D8_PUE = 1;  __asm__("nop");
    D9_PUE = 1;  __asm__("nop");
    D10_PUE = 1; __asm__("nop");
    D12_PUE = 1; __asm__("nop");
    D13_PUE = 1; __asm__("nop");
}

void checkInitialSwitches(void){
    if (D4 == 0)  {initial_switches = (initial_switches |  switch1); delay_by_nop(10); }
    if (D4 == 1)  {initial_switches = (initial_switches & ~switch1); delay_by_nop(10); }
    if (D5 == 0)  {initial_switches = (initial_switches |  switch2); delay_by_nop(10); }
    if (D5 == 1)  {initial_switches = (initial_switches & ~switch2); delay_by_nop(10); }
    if (D6 == 0)  {initial_switches = (initial_switches |  switch3); delay_by_nop(10); }
    if (D6 == 1)  {initial_switches = (initial_switches & ~switch3); delay_by_nop(10); }
    if (D7 == 0)  {initial_switches = (initial_switches |  switch4); delay_by_nop(10); }
    if (D7 == 1)  {initial_switches = (initial_switches & ~switch4); delay_by_nop(10); }
    if (D8 == 0)  {initial_switches = (initial_switches |  switch5); delay_by_nop(10); }
    if (D8 == 1)  {initial_switches = (initial_switches & ~switch5); delay_by_nop(10); }
    if (D9 == 0)  {initial_switches = (initial_switches |  switch6); delay_by_nop(10); }
    if (D9 == 1)  {initial_switches = (initial_switches & ~switch6); delay_by_nop(10); }
    if (D10 == 0) {initial_switches = (initial_switches |  switch7); delay_by_nop(10); }
    if (D10 == 1) {initial_switches = (initial_switches & ~switch7); delay_by_nop(10); }
    if (D13 == 0) {initial_switches = (initial_switches |  switch8); delay_by_nop(10); }
    if (D13 == 1) {initial_switches = (initial_switches & ~switch8); delay_by_nop(10); }
    // if (D13 == 0) {switches = (switches |  switch9); delay_by_nop(10); }
    // if (D13 == 1) {switches = (switches & ~switch9); delay_by_nop(10); }
}


void checkSwitches(void){
    if (D4 == 0)  {switches = (switches |  switch1); delay_by_nop(10); }
    if (D4 == 1)  {switches = (switches & ~switch1); delay_by_nop(10); }
    if (D5 == 0)  {switches = (switches |  switch2); delay_by_nop(10); }
    if (D5 == 1)  {switches = (switches & ~switch2); delay_by_nop(10); }
    if (D6 == 0)  {switches = (switches |  switch3); delay_by_nop(10); }
    if (D6 == 1)  {switches = (switches & ~switch3); delay_by_nop(10); }
    if (D7 == 0)  {switches = (switches |  switch4); delay_by_nop(10); }
    if (D7 == 1)  {switches = (switches & ~switch4); delay_by_nop(10); }
    if (D8 == 0)  {switches = (switches |  switch5); delay_by_nop(10); }
    if (D8 == 1)  {switches = (switches & ~switch5); delay_by_nop(10); }
    if (D9 == 0)  {switches = (switches |  switch6); delay_by_nop(10); }
    if (D9 == 1)  {switches = (switches & ~switch6); delay_by_nop(10); }
    if (D10 == 0) {switches = (switches |  switch7); delay_by_nop(10); }
    if (D10 == 1) {switches = (switches & ~switch7); delay_by_nop(10); }
    if (D13 == 0) {switches = (switches |  switch8); delay_by_nop(10); }
    if (D13 == 1) {switches = (switches & ~switch8); delay_by_nop(10); }
    // if (D13 == 0) {switches = (switches |  switch9); delay_by_nop(10); }
    // if (D13 == 1) {switches = (switches & ~switch9); delay_by_nop(10); }

    // sprintf(buffer, "******************************************");
    // U1_putc('\r'); U1_putc('\n');
    // U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
    // delay_by_nop(600000);
    //
    // sprintf(buffer, "Switches %x  ",
    // switches);
    // U1_putc('\r'); U1_putc('\n');
    // U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
    // delay_by_nop(100);
    //
    //
    // sprintf(buffer, "******************************************");
    // U1_putc('\r'); U1_putc('\n');
    // U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
    // delay_by_nop(600000);
}
