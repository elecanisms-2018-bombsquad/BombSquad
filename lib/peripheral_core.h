#ifndef _PERIPHERAL_CORE_H_
#define _PERIPHERAL_CORE_H_

#include "elecanisms.h"
#include "i2c_reg.h"

extern uint16_t datasend;

typedef enum  {
  STATE_WAIT_FOR_ADDR,
  STATE_WAIT_FOR_WRITE_DATA,
  STATE_SEND_READ_DATA,
  STATE_SEND_READ_LAST
} STATE;


#endif
