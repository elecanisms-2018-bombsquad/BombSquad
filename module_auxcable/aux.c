#include "elecanisms.h"
#include "peripheral_core.h"
#include "i2c_address_space.h"
#include "ajuart.h"
// picocom -b 115200 /dev/ttyUSB0
char buffer[128];

#define MODULE_LED_RED      D0
#define MODULE_LED_GREEN    D1

#define resistor_0 0x3b4
#define resistor_1 0x2ed
#define resistor_2 0x276
#define resistor_3 0x1f1
#define resistor_4 0x162
#define resistor_5 0xc6

#define cutoff_01 850
#define cutoff_12 690
#define cutoff_23 565
#define cutoff_34 430
#define cutoff_45 300

typedef void (*STATE_HANDLER_T)(void); // frame of game states
void setup(void);// forward declaration of module modes
void run(void);
void solved(void);
void end_win(void);
void end_fail(void);
STATE_HANDLER_T state, last_state;

typedef void (*GAME_STATE)(void);  // the states the games goes through
void sixwires(void);
void fivewires(void);
void fourwires(void);
void threewires(void);
void twowires(void);
void onewires(void);
void zerowires(void);
GAME_STATE gamestate;

uint8_t done_striked;

uint16_t a0, a1, a2, a3, a4, a5;
uint8_t wires_out, wires_in;      // counts the number of empty ports

uint16_t aux_cables[6];
uint16_t initial_aux_cables[6];

uint8_t initial_color_vals[7];
uint8_t color_vals[7];

uint16_t winarray[7];

void ledoff(void); // define function
void set_gamestate(void);

void update_analogvals(void);
void initial_aux_values(void);

void update_color_values(void);
void initial_color_values(void);

void update_aux_values(void);
void update_wires_out(void);

void checkwin_cordlocation(void);
void checkwin_cordcolor(void);



// uint8_t check_num_plugged(void){delay_by_nop(1);  }

int16_t main(void) {
    init_elecanisms();
    i2c2_init(157);
    init_ajuart();                      // Initializes I2C on I2C2
    I2C2ADD = MODULE_AUXCABLE_ADDR >>1;   // Set the device address (7-bit register)
    I2C2MSK = 0;                         // Set mask to 0 (only this address matters)
    _SI2C2IE = 1;                        // Enable i2c slave interrupt

    D0_DIR = OUT;
    D1_DIR = OUT;

    state = setup;
    delay_by_nop(300);



    while (1) {
        state();

        sprintf(buffer, "INITIAL |||  AuxVal (0)%x (1)%x (2)%x (3)%x (4)%x (5)%x || ColorVal (1)%x (2)%x (3)%x (4)%x (5)%x (6)%x  ",
        initial_aux_cables[0], initial_aux_cables[1], initial_aux_cables[2], initial_aux_cables[3], initial_aux_cables[4], initial_aux_cables[5],
        initial_color_vals[1], initial_color_vals[2], initial_color_vals[3], initial_color_vals[4], initial_color_vals[5], initial_color_vals[6]
        );
        U1_putc('\r'); U1_putc('\n');
        U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
        delay_by_nop(100);

        sprintf(buffer, "CURRENT |||  AuxVal (0)%x (1)%x (2)%x (3)%x (4)%x (5)%x || ColorVal (1)%x (2)%x (3)%x (4)%x (5)%x (6)%x  ",
        aux_cables[0], aux_cables[1], aux_cables[2], aux_cables[3], aux_cables[4], aux_cables[5],
        color_vals[1], color_vals[2], color_vals[3], color_vals[4], color_vals[5], color_vals[6]
        );
        U1_putc('\r'); U1_putc('\n');
        U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
        delay_by_nop(100);

        sprintf(buffer, "FLAGS   |||  EPS %x | FLUX %x | RTC %x  ", led_eps, led_flux, led_rtc );
        U1_putc('\r'); U1_putc('\n');
        U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
        delay_by_nop(100);

        sprintf(buffer, "-----------------------------------------------");
        U1_putc('\r'); U1_putc('\n');
        U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
        delay_by_nop(600000);


    }
} // end of main

//GAMEPLAY STATE MACHINE FUNCTIONS ////////////////////////////
///////////////////////////////////////////////////////////////
// note that the circle of ports are labeled
// clockwise starting at the top from zero to 5
// this corresponds to the analog read pin associated

void sixwires(void){
    update_analogvals();
    update_aux_values();
    uint8_t twoclockwisewire;
    twoclockwisewire = initial_aux_cables[2];

    winarray[0] = twoclockwisewire;
    winarray[1] = initial_aux_cables[1];
    winarray[2] = 0;
    winarray[3] = initial_aux_cables[3];
    winarray[4] = initial_aux_cables[4];
    winarray[5] = initial_aux_cables[5];

    checkwin_cordlocation();

    // sprintf(buffer, "ACTUAL A0: %x  %x | A1: %x  %x | A2: %x  %x | A3: %x %x  | A4: %x %x  | A5: %x %x  ",
    //                 aux_cables[0],a0,  aux_cables[1],a1, aux_cables[2], a2, aux_cables[3], a3, aux_cables[4], a4, aux_cables[5], a5 );
    // U1_putc('\r'); U1_putc('\n');
    // U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
    // delay_by_nop(100);
    //
    // sprintf(buffer, "GOALS  A0: %x  %x | A1: %x  %x | A2: %x  %x | A3: %x %x  | A4: %x %x  | A5: %x %x  ",
    //                 winarray[0],a0,  winarray[1],a1, winarray[2], a2, winarray[3], a3, winarray[4], a4, winarray[5], a5 );
    // U1_putc('\r'); U1_putc('\n');
    // U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
    // delay_by_nop(2000000);
    // sprintf(buffer, "-----------------------------------------------");
    // U1_putc('\r'); U1_putc('\n');
    // U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
    // delay_by_nop(100);

    sprintf(buffer, "sixwires ---------------------------------------");
    U1_putc('\r'); U1_putc('\n');
    U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
    delay_by_nop(60000);
}

void fivewires(void)  {
// list order is Purple Blue Yellow Red Orange Black
// unplug all wires that are after the one that is already missing

    // purple state, unplug all
    update_color_values();

    if(initial_color_vals[2] == 0){
        winarray[1] = 0;  // black
        winarray[2] = 0;  // purple
        winarray[3] = 0;  // blue
        winarray[4] = 0;  // yellow
        winarray[5] = 0;  // orange
        winarray[6] = 0;  // red
        // sprintf(buffer, "purple state");
        // U1_putc('\r'); U1_putc('\n');
        // U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
        // delay_by_nop(60000);
    }
    // Blue state, remove all but purple
    if(initial_color_vals[3] == 0){
        winarray[1] = 0;  // black
        winarray[2] = 1;  // purple
        winarray[3] = 0;  // blue
        winarray[4] = 0;  // yellow
        winarray[5] = 0;  // orange
        winarray[6] = 0;  // red
        // sprintf(buffer, "blue state");
        // U1_putc('\r'); U1_putc('\n');
        // U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
        // delay_by_nop(60000);
    }
    // Yellow state, remove all but purple, blue
    if(initial_color_vals[4] == 0){
        winarray[1] = 0; // black
        winarray[2] = 1; // purple
        winarray[3] = 1; // blue
        winarray[4] = 0; // yellow
        winarray[5] = 0; // orange
        winarray[6] = 0; // red
        // sprintf(buffer, "yellow state");
        // U1_putc('\r'); U1_putc('\n');
        // U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
        // delay_by_nop(60000);
    }
    // Red state, remove all but purple, blue, yellow
    if(initial_color_vals[6] == 0){
        winarray[1] = 0;  // black
        winarray[2] = 1;  // purple
        winarray[3] = 1;  // blue
        winarray[4] = 1;  // yellow
        winarray[5] = 0;  // orange
        winarray[6] = 0;  // red
        // sprintf(buffer, "red state");
        // U1_putc('\r'); U1_putc('\n');
        // U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
        // delay_by_nop(60000);
    }
    // orange state, remove all but purple, blue, yello, red
    if(initial_color_vals[5] == 0){
        winarray[1] = 0; // black
        winarray[2] = 1; // purple
        winarray[3] = 1; // blue
        winarray[4] = 1; // yellow
        winarray[5] = 0; // orange
        winarray[6] = 1; // red
        // sprintf(buffer, "orange state");
        // U1_putc('\r'); U1_putc('\n');
        // U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
        // delay_by_nop(60000);
    }
    // black state, remove all wires
    if(initial_color_vals[1] == 0){
        winarray[1] = 0; // black
        winarray[2] = 0; // purple
        winarray[3] = 0; // blue
        winarray[4] = 0; // yellow
        winarray[5] = 0; // orange
        winarray[6] = 0; // red
        // sprintf(buffer, "black state");
        // U1_putc('\r'); U1_putc('\n');
        // U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
        // delay_by_nop(60000);
    }
    checkwin_cordcolor();

    sprintf(buffer, "fivewires ---------------------------------------");
    U1_putc('\r'); U1_putc('\n');
    U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
    delay_by_nop(60000);
}


void fourwires(void){
// → If both power and ground are plugged in, remove all of the other wires. (2)
// → If only power is plugged in, attach ground. If only ground is plugged in, attach power.  (5)
// → If neither are plugged in and the EPS alarm is active, plug in power. If EPS is not active, plug in ground. (3)

    update_color_values();

    if(initial_color_vals[6] == 1 && initial_color_vals[1] == 1){
        winarray[1] = 1; // black
        winarray[2] = 0; // purple
        winarray[3] = 0; // blue
        winarray[4] = 0; // yellow
        winarray[5] = 0; // orange
        winarray[6] = 1; // red
    }
    if((initial_color_vals[6] == 1 && initial_color_vals[1] == 0) ||
       (initial_color_vals[6] == 0 && initial_color_vals[1] == 1)){
        winarray[1] = 1; // black
        winarray[2] = color_vals[2]; // purple
        winarray[3] = color_vals[3]; // blue
        winarray[4] = color_vals[4]; // yellow
        winarray[5] = color_vals[5]; // orange
        winarray[6] = 1; // red
    }
    if(initial_color_vals[6] == 0 && initial_color_vals[1] == 0){
        winarray[1] = color_vals[1]; // black
        winarray[2] = color_vals[2]; // purple
        winarray[3] = color_vals[3]; // blue
        winarray[4] = color_vals[4]; // yellow
        winarray[5] = color_vals[5]; // orange
        winarray[6] = color_vals[6]; // red

        if(led_eps == 0){winarray[1] = 1; }
        if(led_eps == 1){winarray[1] = 0; }
    }

    checkwin_cordcolor();

    sprintf(buffer, "four wires ---------------------------------------");
    U1_putc('\r'); U1_putc('\n');
    U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
    delay_by_nop(60000);
}


void threewires(void) {
// Check the status of EPS, FLUX, and RTC.
// If they all have the same status, unplug all of the wires. (0)
// If only one is on, unplug any two of the wires. (1)
// If two are active, unplug any one of the wires. (2)
    uint8_t status_sum;
    status_sum = led_eps + led_flux + led_rtc;

    if(status_sum == 0 || status_sum == 3) {
        LED1 = 1;
        update_wires_out();
        if(wires_out == 6){state = solved; }
    }

    if(status_sum == 1) {
        update_wires_out();
        if(wires_in == 1){state = solved; }
    }

    if(status_sum == 2) {
        update_wires_out();
        if(wires_in == 2){state = solved; }
    }

    sprintf(buffer, "three wires ---------------------------------------");
    U1_putc('\r'); U1_putc('\n');
    U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
    delay_by_nop(60000);

}


void twowires(void) {
// If the two wires are next to each other, plug in all of the wires. (6)
// If they are directly across from one another, unplug them both. (0)
// If neither of those apply, remove one of the wires. (1)
    if(initial_aux_cables[0] != 0 && initial_aux_cables[1] != 0 ||
       initial_aux_cables[1] != 0 && initial_aux_cables[2] != 0 ||
       initial_aux_cables[2] != 0 && initial_aux_cables[3] != 0 ||
       initial_aux_cables[3] != 0 && initial_aux_cables[4] != 0 ||
       initial_aux_cables[4] != 0 && initial_aux_cables[5] != 0 ||
       initial_aux_cables[5] != 0 && initial_aux_cables[0] != 0 ){
           winarray[1] = 1; winarray[2] = 1;
           winarray[3] = 1; winarray[4] = 1;
           winarray[5] = 1; winarray[6] = 1;
           checkwin_cordcolor();
    }

    if(initial_aux_cables[0] != 0 && initial_aux_cables[3] != 0 ||
       initial_aux_cables[1] != 0 && initial_aux_cables[4] != 0 ||
       initial_aux_cables[2] != 0 && initial_aux_cables[5] != 0 ){
           winarray[1] = 0; winarray[2] = 0;
           winarray[3] = 0; winarray[4] = 0;
           winarray[5] = 0; winarray[6] = 0;
           checkwin_cordcolor();
    }
    else{
        update_wires_out();
        if(wires_in == 1) {state = solved; }
    }

    sprintf(buffer, "two wires ---------------------------------------");
    U1_putc('\r'); U1_putc('\n');
    U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
    delay_by_nop(60000);
}




void onewires(void)   {
// If the wire is red, plug in all wires except one. (5)
// If the wire is yellow, plug in all wires except one. (5)
// If the wire is orange, plug in all the wires (6)
// If the wire is blue, plug in all the wires (6)
// If the wire is purple, plug in all the wires except two (4)
// If the wire is black, pull out the wire. (0)

    update_wires_out();
    update_color_values();

    // red or yellow state
    if(initial_color_vals[6] == 1 || initial_color_vals[4] == 1){
        if(wires_in == 5) {state = solved; }
    }

    // orange or blue state
    if(initial_color_vals[5] == 1 || initial_color_vals[3] == 1){
        if(wires_out == 0) {state = solved; }
    }

    // purple state
    if(initial_color_vals[2] == 1 ){
        if(wires_out == 2) {state = solved; }
    }

    // black state
    if(initial_color_vals[1] == 1){
        if(wires_out == 6) {state = solved; }
    }

    sprintf(buffer, "one wires ---------------------------------------");
    U1_putc('\r'); U1_putc('\n');
    U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
    delay_by_nop(60000);

}



void zerowires(void)  {
    uint8_t num_to_add;
    num_to_add = 0;
    if(led_eps == 1) {num_to_add +=1; }
    if(led_rtc == 1) {num_to_add +=2; }
    if(led_flux == 1){num_to_add +=3; }

    if(num_to_add == 0){num_to_add = 6; }

    update_wires_out();
    if(wires_in == num_to_add){state = solved; }

    sprintf(buffer, "zero wires ---------------------------------------");
    U1_putc('\r'); U1_putc('\n');
    U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
    delay_by_nop(60000);
}


// OVERALL GAME STATE MACHINE FUNCTIONS ///////////////////////
///////////////////////////////////////////////////////////////

void setup(void) { // Waits for master module to start the game
    // State Setup
    if (state != last_state) {
        last_state = state;
        MODULE_LED_GREEN = ON; delay_by_nop(1);
        MODULE_LED_RED = ON;
        complete_flag = 0;
        num_strikes = 0;
        error_code = 0;
    }

    // State tasks
    initial_aux_values();
    initial_color_values();
    set_gamestate();

    //Check for state transitions
    if (start_flag == 1) { state = run;}

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
        MODULE_LED_RED = ON;
        done_striked = 0;
    }
    // Perform state tasks
    gamestate();

    // Check for state transitions
    if (win_flag == 1) {state = end_win;}
    else if (lose_flag == 1) {state = end_fail;}

    // State Cleanup
    if (state != last_state) {MODULE_LED_RED = OFF;}
}

void solved(void) { // The puzzle on this module is finished
    // State Setup
    if (state != last_state) {
        last_state = state;
        complete_flag = 1;
        MODULE_LED_GREEN = ON;
    }

    // Perform state tasks
    sprintf(buffer, "---------------You Won!!--------------------");
    U1_putc('\r'); U1_putc('\n');
    U1_puts((uint8_t*)buffer); U1_flush_tx_buffer();
    delay_by_nop(1000);

    // Check for state transitions
    if (win_flag == 1) {
        state = end_win;
    } else if (lose_flag == 1) {
        state = end_fail;
    }

    // State Cleanup
    if (state != last_state) {
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



///////////////////////////////////////////////////////////
////////////////////  Functions  //////////////////////////
///////////////////////////////////////////////////////////


// win check helper functions//////////////////////////////////
///////////////////////////////////////////////////////////////


void checkwin_cordlocation(void){
    update_aux_values();
    if( winarray[0] == aux_cables[0] &&
        winarray[1] == aux_cables[1] &&
        winarray[2] == aux_cables[2] &&
        winarray[3] == aux_cables[3] &&
        winarray[4] == aux_cables[4] &&
        winarray[5] == aux_cables[5] )      {state = solved; }
}

void checkwin_cordcolor(void){
    update_color_values();
    if( winarray[1] == color_vals[1] &&
        winarray[2] == color_vals[2] &&
        winarray[3] == color_vals[3] &&
        winarray[4] == color_vals[4] &&
        winarray[5] == color_vals[5] &&
        winarray[6] == color_vals[6] )    {state = solved; }
}

void set_gamestate(void){
    update_wires_out();     // checks how many wires out  //sets the game mode
    if(wires_out == 0){gamestate = sixwires;   }
    if(wires_out == 1){gamestate = fivewires;  }
    if(wires_out == 2){gamestate = fourwires;  }
    if(wires_out == 3){gamestate = threewires; }
    if(wires_out == 4){gamestate = twowires;   }
    if(wires_out == 5){gamestate = onewires;   }
    if(wires_out == 6){gamestate = zerowires;  }
}

void update_wires_out(void){
    update_aux_values();
    wires_out = 0;
    if(aux_cables[0] == 0) {wires_out += 1;}
    if(aux_cables[1] == 0) {wires_out += 1;}
    if(aux_cables[2] == 0) {wires_out += 1;}
    if(aux_cables[3] == 0) {wires_out += 1;}
    if(aux_cables[4] == 0) {wires_out += 1;}
    if(aux_cables[5] == 0) {wires_out += 1;}
    wires_in = 6 - wires_out;
}

// aux values helper functions/////////////////////////////////
///////////////////////////////////////////////////////////////

void update_analogvals(void){
    a0 = read_analog(A0_AN);
    a1 = read_analog(A1_AN);
    a2 = read_analog(A2_AN);
    a3 = read_analog(A3_AN);
    a4 = read_analog(A4_AN);
    a5 = read_analog(A5_AN);
}

void initial_aux_values(void){
    update_aux_values();
    initial_aux_cables[0] = aux_cables[0];
    initial_aux_cables[1] = aux_cables[1];
    initial_aux_cables[2] = aux_cables[2];
    initial_aux_cables[3] = aux_cables[3];
    initial_aux_cables[4] = aux_cables[4];
    initial_aux_cables[5] = aux_cables[5];
}

void update_aux_values(void){
    update_analogvals();
    if(a0 > cutoff_01) {aux_cables[0] = 1; }
    if(a0 < cutoff_01 && a0 > cutoff_12) {aux_cables[0] = 2; }
    if(a0 < cutoff_12 && a0 > cutoff_23) {aux_cables[0] = 3; }
    if(a0 < cutoff_23 && a0 > cutoff_34) {aux_cables[0] = 4; }
    if(a0 < cutoff_34 && a0 > cutoff_45) {aux_cables[0] = 5; }
    if(a0 < cutoff_45 && a0 > 100) {aux_cables[0] = 6; }
    if(a0 < 100){aux_cables[0] = 0; }

    if(a1 > cutoff_01) {aux_cables[1] = 1; }
    if(a1 < cutoff_01 && a1 > cutoff_12) {aux_cables[1] = 2; }
    if(a1 < cutoff_12 && a1 > cutoff_23) {aux_cables[1] = 3; }
    if(a1 < cutoff_23 && a1 > cutoff_34) {aux_cables[1] = 4; }
    if(a1 < cutoff_34 && a1 > cutoff_45) {aux_cables[1] = 5; }
    if(a1 < cutoff_45 && a1 > 100) {aux_cables[1] = 6; }
    if(a1 < 100){aux_cables[1] = 0; }

    if(a2 > cutoff_01) {aux_cables[2] = 1; }
    if(a2 < cutoff_01 && a2 > cutoff_12) {aux_cables[2] = 2; }
    if(a2 < cutoff_12 && a2 > cutoff_23) {aux_cables[2] = 3; }
    if(a2 < cutoff_23 && a2 > cutoff_34) {aux_cables[2] = 4; }
    if(a2 < cutoff_34 && a2 > cutoff_45) {aux_cables[2] = 5; }
    if(a2 < cutoff_45 && a2 > 100) {aux_cables[2] = 6; }
    if(a2 < 100){aux_cables[2] = 0; }

    if(a3 > cutoff_01) {aux_cables[3] = 1; }
    if(a3 < cutoff_01 && a3 > cutoff_12) {aux_cables[3] = 2; }
    if(a3 < cutoff_12 && a3 > cutoff_23) {aux_cables[3] = 3; }
    if(a3 < cutoff_23 && a3 > cutoff_34) {aux_cables[3] = 4; }
    if(a3 < cutoff_34 && a3 > cutoff_45) {aux_cables[3] = 5; }
    if(a3 < cutoff_45 && a3 > 100) {aux_cables[3] = 6; }
    if(a3 < 100){aux_cables[3] = 0; }

    if(a4 > cutoff_01) {aux_cables[4] = 1; }
    if(a4 < cutoff_01 && a4 > cutoff_12) {aux_cables[4] = 2; }
    if(a4 < cutoff_12 && a4 > cutoff_23) {aux_cables[4] = 3; }
    if(a4 < cutoff_23 && a4 > cutoff_34) {aux_cables[4] = 4; }
    if(a4 < cutoff_34 && a4 > cutoff_45) {aux_cables[4] = 5; }
    if(a4 < cutoff_45 && a4 > 100) {aux_cables[4] = 6; }
    if(a4 < 100){aux_cables[4] = 0; }

    if(a5 > cutoff_01) {aux_cables[5] = 1; }
    if(a5 < cutoff_01 && a5 > cutoff_12) {aux_cables[5] = 2; }
    if(a5 < cutoff_12 && a5 > cutoff_23) {aux_cables[5] = 3; }
    if(a5 < cutoff_23 && a5 > cutoff_34) {aux_cables[5] = 4; }
    if(a5 < cutoff_34 && a5 > cutoff_45) {aux_cables[5] = 5; }
    if(a5 < cutoff_45 && a5 > 100) {aux_cables[5] = 6; }
    if(a5 < 100){aux_cables[5] = 0; }
}

// Cord color helper functions ////////////////////////////////
///////////////////////////////////////////////////////////////

void initial_color_values(void){
    update_color_values();
    // initial_color_vals[0] = color_vals[0];
    initial_color_vals[1] = color_vals[1];
    initial_color_vals[2] = color_vals[2];
    initial_color_vals[3] = color_vals[3];
    initial_color_vals[4] = color_vals[4];
    initial_color_vals[5] = color_vals[5];
    initial_color_vals[6] = color_vals[6];
}

void update_color_values(void){
    update_aux_values();
    if(aux_cables[0]==1||aux_cables[1]==1||aux_cables[2]==1||
       aux_cables[3]==1||aux_cables[4]==1||aux_cables[5]==1){
           color_vals[1] = 1; }
    else { color_vals[1] = 0;}

    if(aux_cables[0]==2||aux_cables[1]==2||aux_cables[2]==2||
       aux_cables[3]==2||aux_cables[4]==2||aux_cables[5]==2){
           color_vals[2] = 1; }
    else { color_vals[2] = 0;}

    if(aux_cables[0]==3||aux_cables[1]==3||aux_cables[2]==3||
       aux_cables[3]==3||aux_cables[4]==3||aux_cables[5]==3){
           color_vals[3] = 1; }
    else { color_vals[3] = 0;}

    if(aux_cables[0]==4||aux_cables[1]==4||aux_cables[2]==4||
       aux_cables[3]==4||aux_cables[4]==4||aux_cables[5]==4){
           color_vals[4] = 1; }
    else { color_vals[4] = 0;}

    if(aux_cables[0]==5||aux_cables[1]==5||aux_cables[2]==5||
       aux_cables[3]==5||aux_cables[4]==5||aux_cables[5]==5){
           color_vals[5] = 1; }
    else { color_vals[5] = 0;}

    if(aux_cables[0]==6||aux_cables[1]==6||aux_cables[2]==6||
       aux_cables[3]==6||aux_cables[4]==6||aux_cables[5]==6){
           color_vals[6] = 1; }
    else { color_vals[6] = 0;}
}

// 0ther helpful helper functions ///////////////////////
///////////////////////////////////////////////////////////////

void ledoff(void) {
    LED1 = 0; delay_by_nop(1);
    LED2 = 0; delay_by_nop(1);
    LED3 = 0; delay_by_nop(1);
    D0 = OFF;
}
