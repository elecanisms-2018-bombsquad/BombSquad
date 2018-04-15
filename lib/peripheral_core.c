#include "peripheral_core.h"
#include "i2c_reg.h"
#include "bs_headers.h"


uint8_t _byte_to_master;
uint8_t _data_from_master;
uint8_t serial_number;
// uint8_t ohshit = 0 ;
uint8_t complete_flag = 0 ;
uint8_t num_strikes = 0 ;
uint8_t error_code = 0;
uint8_t start_flag = 0;
uint8_t win_flag = 0;
uint8_t lose_flag = 0;

volatile I2CSTATE i2c3_state = STATE_WAIT_FOR_ADDR;

void __attribute__((interrupt, auto_psv)) _SI2C3Interrupt(void) {
    uint8_t u8_c;
    _SI2C3IF = 0;

    switch (i2c3_state) {

    case STATE_WAIT_FOR_ADDR:
        u8_c = I2C3RCV;     //clear RBF bit for address
        if (I2C3STATbits.R_W) {   // if data transfer is out from slave // check R/W bit of address byte
            _byte_to_master = (complete_flag << 7) | ((num_strikes & 0b111) << 4) | (error_code & 0b1111) ; // assemble
            I2C3TRN = _byte_to_master;   //Set transmit register to first data byte to send back to master
            I2C3CONbits.SCLREL = 1;    //  SCL released during slave read so master can drive it
            i2c3_state = STATE_END_TRANSACTION; //goes to read transaction
        }
        else i2c3_state = STATE_RECEIVE_FROM_MASTER;     // goes to write transaction
        break;

    case STATE_RECEIVE_FROM_MASTER:     // write transaction state, character arrived, place in buffer
        _data_from_master = I2C3RCV;        //read the byte
        switch (_data_from_master >> 5){
            case HEADER_SERIAL_NUMBER: // 100
                serial_number = _data_from_master & 0b00011111 ;
            break;
            case HEADER_NUM_STRIKES:    // 111
                num_strikes = _data_from_master & 0b00011111 ;
            break;
            case HEADER_START_GAME:    // 000
                start_flag = 1 ;
            break;
            case HEADER_END_WIN:    // 001
                win_flag = 1 ;
            break;
            case HEADER_END_LOSE:    // 010
                lose_flag = 1 ;
            break;

            default: error_code ++ ;

        }
        i2c3_state = STATE_WAIT_FOR_ADDR;    // go to wait for next transaction
        break;

    case STATE_END_TRANSACTION:  //interrupt for last character finished shifting out to master
        i2c3_state = STATE_WAIT_FOR_ADDR;
        break;

    default:
        i2c3_state = STATE_WAIT_FOR_ADDR;

    } // end of switch statements
}   // end of ISR
