// From https://web.archive.org/web/20161223060411/http://www.engscope.com/pic24-tutorial/10-2-i2c-basic-functions/

#ifndef _I2C_REG_H_
#define _I2C_REG_H_

#include "elecanisms.h"

void delay_by_nop(uint32_t num_nops);     //loop nops for delay
void i2c_init(int BRG);                 // initiates I2C3 module to baud rate BRG
void i2c_start(void);                   //function iniates a start condition on bus
void reset_i2c_bus(void);               //Resets the I2C bus to Idle
char send_i2c_byte(int data);           //basic I2C byte send
char i2c_read(void);                    //function reads data, returns the read data, no ack
char i2c_read_ack(void);                //function reads data, returns the read data, with ack
char i2c_read_nack(void); 
void I2Cwrite(char addr, char subaddr, char value);     // function puts together I2C protocol for random write
char I2Cread(char addr, char subaddr);                  // function puts together I2C protocol for random read
unsigned char I2Cpoll(char addr);                       // function checks if device at addr is on bus
unsigned char I2Cwritearray(char addr, char dat[], uint8_t len);    // function writes a byte array over i2c

#endif
