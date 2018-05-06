#ifndef _PERIPHERAL_CORE_H_
#define _PERIPHERAL_CORE_H_

#include "elecanisms.h"
#include "i2c_reg.h"

extern uint16_t datasend;

typedef enum  {
  STATE_WAIT_FOR_ADDR,
  STATE_RECEIVE_FROM_MASTER,
  STATE_END_TRANSACTION,
} I2CSTATE;


extern uint8_t _byte_to_master;
extern uint8_t _data_from_master;
extern uint8_t serial_number;
extern uint8_t led_eps;
extern uint8_t led_flux;
extern uint8_t led_rtc;
// extern // uint8_t ohshit ;
extern uint8_t complete_flag ;
extern uint8_t num_strikes ;
extern uint8_t error_code;
extern uint8_t start_flag;
extern uint8_t win_flag;
extern uint8_t lose_flag;


#endif
