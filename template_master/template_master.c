#include "elecanisms.h"
#include "i2c_reg.h"
#include "ajuart.h"

#define SLAVE_ADDR 0x60

uint8_t data_buffer[1024];

uint8_t datareturned;

void ledoff(void) {
    LED1 = 0;delay_by_nop(1);
    LED2 = 0; delay_by_nop(1);
    LED3 = 0; delay_by_nop(1);
    D0 = OFF;
}

int main(void) {
    init_elecanisms();
    init_ajuart();
    uint16_t u16_len;
    uint16_t pollval;
    int16_t i = 0;
    i2c2_init(157);      // initialize I2C for 16Mhz OSC with 100kHz I2C clock

    delay_by_nop(3000);

    D0_DIR = OUT;

    delay_by_nop(300000);
    while(1) {
        datareturned = 0;
        delay_by_nop(500);
        i2c2_start();
        send_i2c2_byte(SLAVE_ADDR | 1);  // init a read, last to 1
        datareturned = i2c2_read_nack();
        if(datareturned > 1) {LED1 = 1;}
        if(datareturned == 0b10000000) {D0 = ON; delay_by_nop(1);}
        if(datareturned == 0b00000000) {LED3 = 1;delay_by_nop(1);}
        reset_i2c2_bus();

        U1_putc(datareturned);
        U1_flush_tx_buffer();

        delay_by_nop(1000000);
        ledoff();
        delay_by_nop(1000000);

    }
}
