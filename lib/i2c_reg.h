// From https://web.archive.org/web/20161223060411/http://www.engscope.com/pic24-tutorial/10-2-i2c-basic-functions/

#ifndef _I2C_REG_H_
#define _I2C_REG_H_

#include "elecanisms.h"

//loop nops for delay
void blocking_delay_us(uint16_t N);
// initiates I2C1 module to baud rate BRG
void i2c_init(int BRG);
//function iniates a start condition on bus
void i2c_start(void);
//Resets the I2C bus to Idle
void reset_i2c_bus(void);
//basic I2C byte send
char send_i2c_byte(int data);
//function reads data, returns the read data, no ack
char i2c_read(void);
//function reads data, returns the read data, with ack
char i2c_read_ack(void);
// function puts together I2C protocol for random write
void I2Cwrite(char addr, char subaddr, char value);
// function puts together I2C protocol for random read
char I2Cread(char addr, char subaddr);
// function checks if device at addr is on bus
unsigned char I2Cpoll(char addr);
// function writes a byte array over i2c
unsigned char I2Cwritearray(char addr, char dat[], uint8_t len);

#endif
