#include <p24FJ128GB206.h>
#include <stdint.h>
#include "elecanisms.h"
#include "i2c_reg.h"

#define PIN_ENCODER_A 0
#define PIN_ENCODER_B 2

static uint8_t enc_prev_pos = 0;
static uint8_t enc_flags    = 0;

void setup(){
  // set pins as input with internal pull-up resistors enabled
  pinMode(PIN_ENCODER_A, INPUT);
  pinMode(PIN_ENCODER_B, INPUT);
  digitalWrite(PIN_ENCODER_A, HIGH);
  digitalWrite(PIN_ENCODER_B, HIGH);

  // TrinketHidCombo.begin(); // start the USB device engine and enumerate

  // get an initial reading on the encoder pins
  if (digitalRead(PIN_ENCODER_A) == LOW) {enc_prev_pos |= (1 << 0);}
  if (digitalRead(PIN_ENCODER_B) == LOW) {enc_prev_pos |= (1 << 1);}
}

void loop(){
  int8_t enc_action = 0; // 1 or -1 if moved, sign is direction
  uint8_t enc_cur_pos = 0;

  if (bit_is_clear(TRINKET_PINx, PIN_ENCODER_A)) {enc_cur_pos |= (1 << 0);}// read in the encoder state first
  if (bit_is_clear(TRINKET_PINx, PIN_ENCODER_B)) {enc_cur_pos |= (1 << 1);}

  // if any rotation at all
  if (enc_cur_pos != enc_prev_pos){
    if (enc_prev_pos == 0x00) {
      if (enc_cur_pos == 0x01) {enc_flags |= (1 << 0); }        // this is the first edge
      else if (enc_cur_pos == 0x02) {enc_flags |= (1 << 1);}
    }

    if (enc_cur_pos == 0x03){enc_flags |= (1 << 4); }   // this is when the encoder is in the middle of a "step"
    else if (enc_cur_pos == 0x00){
      if (enc_prev_pos == 0x02) {enc_flags |= (1 << 2);}     // this is the final edge
      else if (enc_prev_pos == 0x01) {enc_flags |= (1 << 3);}

      // check the first and last edge
      // or maybe one edge is missing, if missing then require the middle state
      // this will reject bounces and false movements
      if (bit_is_set(enc_flags, 0) && (bit_is_set(enc_flags, 2) || bit_is_set(enc_flags, 4))) {
        enc_action = 1;}

      else if (bit_is_set(enc_flags, 2) && (bit_is_set(enc_flags, 0) || bit_is_set(enc_flags, 4))) {
        enc_action = 1;}

      else if (bit_is_set(enc_flags, 1) && (bit_is_set(enc_flags, 3) || bit_is_set(enc_flags, 4))) {
        enc_action = -1;}

      else if (bit_is_set(enc_flags, 3) && (bit_is_set(enc_flags, 1) || bit_is_set(enc_flags, 4))) {
        enc_action = -1;}

      enc_flags = 0; // reset for next time

    }
  }

  enc_prev_pos = enc_cur_pos;

  if (enc_action > 0) {TrinketHidCombo.pressMultimediaKey(MMKEY_VOL_UP);}
  else if (enc_action < 0) {TrinketHidCombo.pressMultimediaKey(MMKEY_VOL_DOWN);}
  else {TrinketHidCombo.poll(); // do nothing, check if USB needs anything done}
}
