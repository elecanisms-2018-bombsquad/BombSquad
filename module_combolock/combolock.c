// #include <p24FJ128GB206.h>
// #include <stdint.h>

#include "elecanisms.h"
#include "i2c_reg.h"
//#include "adafruit_led.h"


//
//
#define PIN_ENCODER_A D9
#define PIN_ENCODER_B D1  /* Remap these to some digital pins from elecanisms.h*/
//
static uint8_t enc_prev_pos = 0;
static uint8_t enc_flags    = 0;
static uint8_t screen_pos = 0; //what position the active character is on.
static uint8_t UP = 1;
static uint8_t DOWN = 0;

volatile uint16_t current_led = 0;
//

void __attribute__((interrupt, auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    switch (current_led) {  // Toggle whichever LED is the current one
      case 1 :
        LED1 = !LED1;
        LED2 = 0;
        break;
      default :
        LED2 = !LED2;
        LED1 = 0;
        break;
    }
}

void __attribute__((interrupt, auto_psv)) _T2Interrupt(void) {
    IFS0bits.T2IF = 0;      // lower Timer2 interrupt flag
    T2CONbits.TON = 0;      // turn off timer
    if (PORTDbits.RD10 == 1) {          // Sample D9 after debounce
      current_led ^= 1;
    }
}

void __attribute__((interrupt, auto_psv)) _INT3Interrupt(void) {
    IFS3bits.INT3IF = 0;      // lower INT3 interrupt flag
    TMR2 = 0;                 // reset debounce Timer2
    IFS0bits.T2IF = 0;        // lower Timer2 Interrupt flag
    T2CONbits.TON = 1;        // start Timer2
}

int16_t main(void){
    init_elecanisms();
    // set pins as input with internal pull-up resistors enabled
    //pinMode -> use elecanisms.h, use the function i.e. D0_DIR = OUT
    D0_DIR = IN; /* This is an arduino function, use DX_DIR = X; */
    D1_DIR = IN;
    PIN_ENCODER_A = ON; /* This is an arduino function, use DX = ON/OFF; */
    PIN_ENCODER_B = ON;

    //Debug
    LED1 = OFF;

    //Copied the debounce/timer stuff from blink.c

    // Timer 1 Setup
        T1CON = 0x0030;         // set Timer1 period to 0.25s, prescaler 256 match 15624
        PR1 = 0x3D08;

        TMR1 = 0;               // set Timer1 count to 0
        IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
        IEC0bits.T1IE = 1;      // enable Timer1 interrupt
        T1CONbits.TON = 1;      // turn on Timer1

    // Timer 2 Setup
        T2CON = 0x0020;         // set Timer2 period to 10 ms for debounce
        PR2 = 0x2710;           // prescaler 16, match value 10000

        TMR2 = 0;               // set Timer2 to 0
        IFS0bits.T2IF = 0;      // lower T2 interrupt flag
        IEC0bits.T2IE = 1;      // enable T2 interrupt
        T2CONbits.TON = 0;      // make sure T2 isn't on

    // INT3 Setup
        RPINR1bits.INT3R = INT3_RP; // Configure interrupt 3 on RP3, pin D9 on board

        IFS3bits.INT3IF = 0;    // lower interrupt flag for INT3
        IEC3bits.INT3IE = 1;    // enable INT3 interrupt

    // get an initial reading on the encoder pins
    if (PIN_ENCODER_A == 0) {enc_prev_pos |= (1 << 0);} /* Also arduino */
    if (PIN_ENCODER_B == 0) {enc_prev_pos |= (1 << 1);}

    while (1){
      loop();
    }
}

void SelectChr(){//Locks in the current character on the screen.

}

void test_LED(){
  while (1){LED1 = ON;}
}

void dispChr(uint8_t direction){//Increments/decrements the character on the screen.
  if (direction == UP){
    if (LED1 == ON){LED1 = OFF;}
    else {LED1 = ON;}
    }
  else if (direction == DOWN){
    if (LED3 == ON){LED3 = OFF;}
    else {LED3 = ON;}
    }
  else {;}
}

//
void loop(){
  int8_t enc_action = 0; // 1 or -1 if moved, sign is direction
  uint8_t enc_cur_pos = 0;
//

  //Ground state of the pins is 1.  Therefore 0 means a change.
  /*Can use A as the clock.  Whenever a rising edge of A is detected, check state of B.
  So when A is "0", if B is 0, then that means it's one direction, and if B is one, then
  it's another direction.


  Add in interrupt so can control the system through that instead of polling.
  /*
  if (PIN_ENCODER_A == 0){
    enc_cur_pos |= (1 <<0);

    } //Read Encoder_Pin_A?
  if (PIN_ENCODER_B == 0){enc_cur_pos |= (1 <<1);} //Read Encoder_Pin_B?
  //if (enc_cur_pos != 0){test_LED();}
  */




//   // if any rotation at all
  // if (enc_cur_pos != enc_prev_pos){
  //   if (enc_prev_pos == 0x00) {
  //     if (enc_cur_pos == 0x01) {enc_flags |= (1 << 0);}        // this is the first edge
  //     else if (enc_cur_pos == 0x02) {enc_flags |= (1 << 1);}
  //   }
  //
  //   if (enc_cur_pos == 0x03){enc_flags |= (1 << 4);}   // this is when the encoder is in the middle of a "step"
  //   else if (enc_cur_pos == 0x00){
  //     if (enc_prev_pos == 0x02) {enc_flags |= (1 << 2);}     // this is the final edge
  //     else if (enc_prev_pos == 0x01) {enc_flags |= (1 << 3);}
  //   }
  //}
// check the first and last edge
// or maybe one edge is missing, if missing then require the middle state
// this will reject bounces and false movements

// //       if (bit_is_set(enc_flags, 0) && (bit_is_set(enc_flags, 2) || bit_is_set(enc_flags, 4))) {
// //         enc_action = 1;}
//
//   if ((enc_flags & (1 << 0)) && enc_flags & (1 << 2) || (enc_flags & (1 << 4))){
//         enc_action = 1;
//   }
// //
// //       else if (bit_is_set(enc_flags, 2) && (bit_is_set(enc_flags, 0) || bit_is_set(enc_flags, 4))) {
// //         enc_action = 1;}
//
//   else if ((enc_flags & (1 << 2)) && enc_flags & (1 << 0) || (enc_flags & (1 << 4))){
//     enc_action = 1;
//   }
// //
// //       else if (bit_is_set(enc_flags, 1) && (bit_is_set(enc_flags, 3) || bit_is_set(enc_flags, 4))) {
// //         enc_action = -1;}
//
//   else if ((enc_flags & (1 << 1)) && enc_flags & (1 << 3) || (enc_flags & (1 << 4))){
//       enc_action = -1;
//   }
//
// //       else if (bit_is_set(enc_flags, 3) && (bit_is_set(enc_flags, 1) || bit_is_set(enc_flags, 4))) {
// //         enc_action = -1;}
//
//   else if ((enc_flags & (1 << 3)) && enc_flags & (1 << 1) || (enc_flags & (1 << 4))){
//       enc_action = 1;
//   }
// //
//   enc_flags = 0; // reset for next time
//
//     }
//   }
//   //Encoder action is basically going up or down.  So basically can use that to increment up and down.
//
//   // Figure out the output from the loop, detect output and convert it to an incrementing table.
//   // Interrupt to select the current character and move on to selecting the next character.


  enc_prev_pos = enc_cur_pos;


  //if (enc_action > 0) {dispChr(UP);}
  //else if (enc_action < 0) {dispChr(DOWN);}
}
//
//
