#include <p24FJ128GB206.h>
#include <stdint.h>

#include "common.h" // common.h needed for _init_clock
#include "elecanisms.h"
#include "lcd.h"


// I2C Reg (MSB) P7 P6 P5 P4 P3 P2 P1 P0
// Driver pin    D7 D6 D5 D4 ?  E  RW RS

int16_t main(void) {
    init_clock();

    // Initializes I2C on I2C3
    i2c_init(1e3);

    // Initializes LCD structs with addresses
    _LCD lcd1, lcd2, lcd3;
    lcd_init(&lcd1, 0x06, 'A');
    lcd_init(&lcd2, 0x07, 'A');
    lcd_init(&lcd3, 0x05, 'A');

    lcd_clear(&lcd1);  // Clears _LCD objects from previous array
    lcd_clear(&lcd2);
    lcd_clear(&lcd3);

    char string1[17]="Disp1Line1";
    char* strptr1=string1;
    char string2[17]="Disp1Line2";
    char* strptr2=string2;
    char string3[17]="Disp2Line1";
    char* strptr3=string3;
    char string4[17]="Disp2Line2";
    char* strptr4=string4;
    char string5[17]="Disp3Line1";
    char* strptr5=string5;
    char string6[17]="Disp3Line2";
    char* strptr6=string6;

    lcd_print2(&lcd1,strptr1,strptr2);
    lcd_print2(&lcd2,strptr3,strptr4);
    lcd_print2(&lcd3,strptr5,strptr6);

    while(1) {
        //
    }
}
