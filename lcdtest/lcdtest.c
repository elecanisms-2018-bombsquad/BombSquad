#include <p24FJ128GB206.h>
#include <stdint.h>

#include "common.h" // common.h needed for _init_clock

#include "i2c.h"
#include "timer.h"
#include "lcd.h"


// I2C Reg (MSB) P7 P6 P5 P4 P3 P2 P1 P0
// Driver pin    D7 D6 D5 D4 ?  E  RW RS

int16_t main(void) {
    init_clock();
    init_ui();
    init_pin();
    init_uart();
    init_i2c();
    init_timer();
    timer_initDelayMicro(&timer5);

    init_lcd(3);

    lcd_clear(&lcd[0]);
    lcd_clear(&lcd[1]);
    lcd_clear(&lcd[2]);


    printf("____________\r\n");
    char string1[17]="Spark Scrambler";
    char* strptr1=string1;
    char string2[17]="Suction Scissor";
    char* strptr2=string2;
    char string3[17]="Photo Sprinkler";
    char* strptr3=string3;
    char string4[17]="Epsilon Counter";
    char* strptr4=string4;
    char string5[17]="Depth Sanitizer";
    char* strptr5=string5;
    char string6[17]="Ice Thrower";
    char* strptr6=string6;


    strm_Scramble(strptr1,2,10);
    strm_Scramble(strptr2,2,10);
    strm_Scramble(strptr3,2,10);
    strm_Scramble(strptr4,2,10);
    strm_Scramble(strptr5,2,10);
    strm_Scramble(strptr6,2,10);
    lcd_print2(&lcd[0],strptr1,strptr2);
    lcd_print2(&lcd[1],strptr3,strptr4);
    lcd_print2(&lcd[2],strptr5,strptr6);

    char numbers[33][3]={"0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23","24","25","26","27","28","29","30","31","32"};


    while(1) {
        //
    }
}
