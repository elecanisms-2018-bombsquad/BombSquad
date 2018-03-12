#include <p24FJ128GB206.h>
#include <stdint.h>

#include "common.h" // common.h needed for _init_clock
#include "elecanisms.h"
#include "lcd.h"


// I2C Reg (MSB) P7 P6 P5 P4 P3 P2 P1 P0
// Driver pin    D7 D6 D5 D4 ?  E  RW RS

int16_t main(void) {
    init_clock();

    init_lcd(3); // Inits 3 _LCD type objects in an array


    lcd_clear(&lcd[0]);  // Clears _LCD objects from previous array
    lcd_clear(&lcd[1]);
    lcd_clear(&lcd[2]);


    char string1[17]="Testing i2creg";
    char* strptr1=string1;
    char string2[17]="using old lcd.h";
    char* strptr2=string2;
    char string3[17]="Photo Sprinkler";
    char* strptr3=string3;
    char string4[17]="Epsilon Counter";
    char* strptr4=string4;
    char string5[17]="Depth Sanitizer";
    char* strptr5=string5;
    char string6[17]="Ice Thrower";
    char* strptr6=string6;

    lcd_print2(&lcd[1],strptr1,strptr2);
    lcd_print2(&lcd[0],strptr3,strptr4);

    while(1) {
        //
    }
}
