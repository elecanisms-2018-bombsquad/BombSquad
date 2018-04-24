#include "elecanisms.h"
#include "i2c_reg.h"
#include "ajuart.h"
#include "i2c_address_space.h"
#include "bs_headers.h"

uint8_t data_buffer[1024];

uint8_t datareturned;

uint8_t peripheral_addrs[6] = {TEST_PERIPHERAL_ADDR,
                               MODULE_CODEWORD_ADDR,
                               MODULE_AUXCABLE_ADDR,
                               MODULE_BUTTON_ADDR  ,
                               MODULE_NEEDY_ADDR   ,
                               MODULE_SIMON_ADDR   };

void ledoff(void) {
    LED1 = 0;delay_by_nop(1);
    LED2 = 0; delay_by_nop(1);
    LED3 = 0; delay_by_nop(1);
    D0 = OFF;
}

int main(void) {
    init_elecanisms();
    init_ajuart();
    uint8_t peripheral_present[6] = {0,0,0,0,0,0};
    uint8_t peripheral_complete[6] = {0,0,0,0,0,0};
    uint8_t num_strikes = 0;
    uint8_t prev_num_strikes = 0;
    uint8_t game_complete = 0;

    i2c2_init(157);      // initialize I2C for 16Mhz OSC with 100kHz I2C clock


    delay_by_nop(3000);

    D0_DIR = OUT;

    delay_by_nop(300000);


    uint8_t i = 0;
    // Poll the peripherals to see who's here
    for (i = 0; i < 6; i++) {
        uint8_t temp = 0;

        i2c2_start();
        temp = send_i2c2_byte(peripheral_addrs[i]); // set /W bit
        if (temp == 0) {
            peripheral_present[i] = 1; // We found it, but we need to reset the FSM for i2c
            send_i2c2_byte(0xA0); // Send dummy byte to reset FSM
        }
        reset_i2c2_bus();
    }

    //TODO: Send out parameters

    //TODO: Send out start condition
    for (i = 0; i < 6; i++) {
        if (peripheral_present[i]) {
            i2c2_start();
            send_i2c2_byte(peripheral_addrs[i] | 0);  // init a write, last to 0
            send_i2c2_byte(HEADER_START_GAME << 5);
            reset_i2c2_bus();
        }
    }

    while(1) {
        datareturned = 0;
        delay_by_nop(500);
        LED3 = ON;

    //TODO: Abstract out this section in a state named 'run'

        //TODO: Handle time

        // Get completeness and strikes from every module
        prev_num_strikes = num_strikes;
        for (i = 0; i < 6; i++) {
            if(peripheral_present[i]) {
                i2c2_start();
                send_i2c2_byte(peripheral_addrs[i] | 1);  // init a read, last to 1
                datareturned = i2c2_read_nack();
                reset_i2c2_bus();

                if (datareturned & 0b10000000) { // Complete flag
                    peripheral_complete[i] = 1;
                }
                if (((datareturned & 0b01110000) >> 4) > prev_num_strikes) { //If the module recorded any strikes
                    num_strikes+= ((datareturned & 0b01110000) >> 4);
                }
                if ((datareturned & 0b00001111) != 0) {
                    // TODO: implement error codes if necessary
                }
            }
        }
        //Handles completeness
        game_complete = 1;
        for (i = 0; i < 6; i++) {
            if (peripheral_present[i] && !peripheral_complete[i]) {
                game_complete = 0;
            }
        }
        // If we checked all of them and the game is still complete, then count it!
        if (game_complete) {
            for (i = 0; i < 6; i++) {
                if(peripheral_present[i]) {
                    i2c2_start();
                    send_i2c2_byte(peripheral_addrs[i] | 0);  // init a write, last to 0
                    send_i2c2_byte(HEADER_END_WIN << 5); // Broadcast that we won
                    reset_i2c2_bus();
                }
            }
            //TODO: Go to 'win' state
        }

        //Handles strikes
        if (num_strikes > 2) {
            for (i = 0; i < 6; i++) {
                if(peripheral_present[i]) {
                    i2c2_start();
                    send_i2c2_byte(peripheral_addrs[i] | 0);  // init a write, last to 0
                    send_i2c2_byte(HEADER_END_LOSE << 5); // Broadcast the current number of strikes
                    reset_i2c2_bus();
                }
            }
            //TODO: Go to 'fail' state
        }
        else if (num_strikes > prev_num_strikes) {
            for (i = 0; i < 6; i++) {
                if (peripheral_present[i]) {
                    i2c2_start();
                    send_i2c2_byte(peripheral_addrs[i] | 0);  // init a write, last to 0
                    send_i2c2_byte((HEADER_NUM_STRIKES<<5) | num_strikes); // Broadcast the current number of strikes
                    reset_i2c2_bus();
                }
            }
        }

        // char str[64] = "Message received from peripheral:";
        // U1_puts(str);
        // U1_putc(datareturned);
        // U1_putc('\r');
        // U1_putc('\n');
        // U1_flush_tx_buffer();

        delay_by_nop(1000000);
        ledoff();
        delay_by_nop(1000000);

    }
}
