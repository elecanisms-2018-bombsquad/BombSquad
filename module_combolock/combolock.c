// #include <p24FJ128GB206.h>
// #include <stdint.h>

#include "elecanisms.h"
#include "i2c_reg.h"
//#include "adafruit_led.h"


//
//
#define PIN_ENCODER_A D0
#define PIN_ENCODER_B D1  /* Remap these to some digital pins from elecanisms.h*/
//
static uint8_t enc_prev_pos = 0;
static uint8_t enc_flags    = 0;
static uint8_t screen_pos = 0; //what position the active character is on.
static uint8_t UP = 1;
static uint8_t DOWN = 0;
//
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

    T1CON = 0x0030;         // set Timer1 period to 0.5s
    PR1 = 0x7A11;

    TMR1 = 0;               // set Timer1 count to 0
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    IEC0bits.T1IE = 1;      // enable Timer1 interrupt
    T1CONbits.TON = 1;      // turn on Timer1

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
//   /*The original code referenced a bitwise connection between "Trinket_PINx" (defined as "PINB") and also
//   PIN_ENCODER whatever.  What the hell is PINB?*/

//if (bit_is_clear(TRINKET_PINx, PIN_ENCODER_A)) {enc_cur_pos |= (1 << 0);}// read in the encoder state first
//if (bit_is_clear(TRINKET_PINx, PIN_ENCODER_B)) {enc_cur_pos |= (1 << 1);}  /* Arduino function as well */


  //Ground state of the pins is 1.  Therefore 0 means a change.
  /*Can use A as the clock.  Whenever a rising edge of A is detected, check state of B.
  So when A is "0", if B is 0, then that means it's one direction, and if B is one, then
  it's another direction.


  Add in interrupt so can control the system through that instead of polling.  
  */
  if (PIN_ENCODER_A == 0){
    enc_cur_pos |= (1 <<0);
    test_LED();
    } //Read Encoder_Pin_A?
  if (PIN_ENCODER_B == 0){enc_cur_pos |= (1 <<1);} //Read Encoder_Pin_B?
  //if (enc_cur_pos != 0){test_LED();}



//   // if any rotation at all
  if (enc_cur_pos != enc_prev_pos){
    if (enc_prev_pos == 0x00) {
      if (enc_cur_pos == 0x01) {enc_flags |= (1 << 0);}        // this is the first edge
      else if (enc_cur_pos == 0x02) {enc_flags |= (1 << 1);}
    }

    if (enc_cur_pos == 0x03){enc_flags |= (1 << 4);}   // this is when the encoder is in the middle of a "step"
    else if (enc_cur_pos == 0x00){
      if (enc_prev_pos == 0x02) {enc_flags |= (1 << 2);}     // this is the final edge
      else if (enc_prev_pos == 0x01) {enc_flags |= (1 << 3);}
    }
  }
// check the first and last edge
// or maybe one edge is missing, if missing then require the middle state
// this will reject bounces and false movements

//       if (bit_is_set(enc_flags, 0) && (bit_is_set(enc_flags, 2) || bit_is_set(enc_flags, 4))) {
//         enc_action = 1;}

  if ((enc_flags & (1 << 0)) && enc_flags & (1 << 2) || (enc_flags & (1 << 4))){
        enc_action = 1;
  }
//
//       else if (bit_is_set(enc_flags, 2) && (bit_is_set(enc_flags, 0) || bit_is_set(enc_flags, 4))) {
//         enc_action = 1;}

  else if ((enc_flags & (1 << 2)) && enc_flags & (1 << 0) || (enc_flags & (1 << 4))){
    enc_action = 1;
  }
//
//       else if (bit_is_set(enc_flags, 1) && (bit_is_set(enc_flags, 3) || bit_is_set(enc_flags, 4))) {
//         enc_action = -1;}

  else if ((enc_flags & (1 << 1)) && enc_flags & (1 << 3) || (enc_flags & (1 << 4))){
      enc_action = -1;
  }

//       else if (bit_is_set(enc_flags, 3) && (bit_is_set(enc_flags, 1) || bit_is_set(enc_flags, 4))) {
//         enc_action = -1;}

  else if ((enc_flags & (1 << 3)) && enc_flags & (1 << 1) || (enc_flags & (1 << 4))){
      enc_action = 1;
  }
//
  enc_flags = 0; // reset for next time
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
