#include <p24FJ128GB206.h>

// #include "common.h"
#include "lcd.h"
#include "timer.h"
// #include "ui.h"

// I2C Reg (MSB) P7 P6 P5 P4 P3 P2 P1 P0
// Driver pin    D7 D6 D5 D4 ?  E  RW RS

//
#define LCD_FUNCTIONSET 0x20
#define DISPLAYMASK 0x08
#define ENABLE_TOGGLE 0x04
#define LCD_BACKLIGHT 0x08

// Commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// Flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// Flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// Flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// Modes
#define INTERNAL_WRITE 0x08
#define BUSY_READ 0x0A
#define DR_WRITE 0x09
#define DR_READ 0x0B

_LCD lcd[3];

_I2C *__lcd_i2c;

void __lcd_i2c_write(_LCD *self, uint8_t ch) {
    i2c_start(__lcd_i2c);
    i2c_putc(__lcd_i2c, self->addr_write);
    i2c_idle(__lcd_i2c);
    i2c_putc(__lcd_i2c, ch);
    i2c_idle(__lcd_i2c);
    i2c_stop(__lcd_i2c);
}

void __lcd_enablePulse(_LCD *self) {
    self->io_write_val ^= ENABLE_TOGGLE;
    __lcd_i2c_write(self, self->io_write_val);
    timer_delayMicro(100);
    self->io_write_val ^= ENABLE_TOGGLE;
    __lcd_i2c_write(self, self->io_write_val);
    // timer_delayMicro(1000);
}

void __lcd_send(_LCD *self, uint8_t value, uint8_t command) {
    uint8_t MS = value & 0x78;
    uint8_t LS = value << 4;
    self->io_write_val = command | MS;
    __lcd_i2c_write(self, self->io_write_val);
    __lcd_enablePulse(self);
    self->io_write_val= command | LS;
    __lcd_i2c_write(self, self->io_write_val);
    __lcd_enablePulse(self);
}

void __lcd_send8(_LCD *self, uint8_t value, uint8_t command) {
    value = value << 4;
    self->io_write_val = command | value;
    __lcd_i2c_write(self, self->io_write_val);
    __lcd_enablePulse(self);
}


void init_lcd(uint8_t initiator) {
    __lcd_i2c = &i2c3;
    i2c_open(__lcd_i2c, 1e3);

    switch (initiator) {
        case 0: // Central
            lcd_init(&lcd[0], 0x05,'A');
            lcd_init(&lcd[1], 0x07,'A');
            lcd_init(&lcd[2], 0x06,'T');
            break;
        case 1:
            lcd_init(&lcd[0], 0x07,'A');
            lcd_init(&lcd[2], 0x06,'A');
            lcd_init(&lcd[1], 0x05,'A');
            break;
        case 2:
            lcd_init(&lcd[0], 0x07,'T');
            lcd_init(&lcd[1], 0x06,'A');
            lcd_init(&lcd[2], 0x05,'A');
            break;
        case 3:
            lcd_init(&lcd[0], 0x07,'A');
            lcd_init(&lcd[1], 0x06,'A');
            lcd_init(&lcd[2], 0x05,'A');
            break;
    }
}

void lcd_init(_LCD *self, uint8_t addr, char vendor) {
    switch(vendor){
        case 'T':// 0x40 == vendor prefix for PCF8574T
            self->addr_write = 0x40 + (addr << 1);
            self->addr_read = 0x40 + (addr << 1)+1;
            break;
        case 'A':// 0x70 == vendor prefix PCF8574AT
            self->addr_write = 0x70 + (addr << 1);
            self->addr_read = 0x70 + (addr << 1)+1;
            break;
    }

    self->display_control = 0x00;
    self->display_mode = 0x00;

    self->io_write_val = 0x00;

    __lcd_i2c_write(self, 0x00);

    timer_delayMicro(15000);

    // Some bullshit according to pg 46
    __lcd_send8(self, 0x03, INTERNAL_WRITE);
    timer_delayMicro(5000);

    __lcd_send8(self, 0x03, INTERNAL_WRITE);//0b00110000
    timer_delayMicro(5000);

    __lcd_send8(self, 0x03, INTERNAL_WRITE);//0b00110000
    timer_delayMicro(5000);

    // Put it in 4 bit mode
    __lcd_send8(self, 0x02, INTERNAL_WRITE);//0b00110000
    timer_delayMicro(5000);

    __lcd_send(self, 0x28, INTERNAL_WRITE); // Set rows and direction
    timer_delayMicro(50);

    __lcd_send(self, 0x80, INTERNAL_WRITE); // Display off, cursor off
    timer_delayMicro(50);

    __lcd_send(self, 0x01, INTERNAL_WRITE); // Go to home position
    timer_delayMicro(2000);

    __lcd_send(self, 0x06, INTERNAL_WRITE); // Set curson direction
    timer_delayMicro(5000);

    __lcd_send(self, 0x0C, INTERNAL_WRITE); // Display on, cursor off
}

void lcd_stop(_LCD *self) {
    i2c_stop(__lcd_i2c);
}

void lcd_display(_LCD *self, uint8_t on) {
    if (on) {
        self->display_control |= LCD_DISPLAYON;
    } else {
        self->display_control &= ~LCD_DISPLAYON;
    }
    __lcd_send(self, self->display_control | LCD_DISPLAYCONTROL, INTERNAL_WRITE);
}

void lcd_clear(_LCD *self) {
    __lcd_send(self, LCD_CLEARDISPLAY, INTERNAL_WRITE);
    timer_delayMicro(2000);
}

void lcd_putc(_LCD *self, char c) {
    __lcd_send(self, c, DR_WRITE);
    // timer_delayMicro(1000);
}

void lcd_goto(_LCD *self, uint8_t line, uint8_t col) { //x=col, y=row
    uint8_t address;
    switch(line) {
        case 1:
            address = 0x00;
            break;
        case 2:
            address = 0x40;
            break;
        default:
            address = 0x00;
            break;
    }

    address = address+col;
    __lcd_send(self, LCD_SETDDRAMADDR | address, INTERNAL_WRITE);
}

void lcd_cursor(_LCD *self, uint8_t cur) {
    switch(cur) {
        case 0:
            __lcd_send(self, 0x0C, INTERNAL_WRITE);
            break;
        case 1:
            __lcd_send(self, 0x0E, INTERNAL_WRITE);
            break;
        default:
            break;
    }
}

void lcd_print1(_LCD *self, char *str) {
    lcd_clear(self);
    while (*str) {
        lcd_putc(self, *str);
        str++;
    }
}

void lcd_print2(_LCD *self, char* line1, char* line2){
    lcd_clear(self);
    char str[56] ="                                                        ";
    int i =0;
    while (*line1){
        str[i]=*line1;
        i=i+1;
        line1++;
    }
    i =40;
    while (*line2){
        str[i]=*line2;
        i=i+1;
        line2++;
    }
    char* strptr=str;
    lcd_print1(self,strptr);
}

void lcd_print(_LCD *self, char* message) {
    char newstr1[17] = "                ";
    char* newstrptr1= newstr1;
    char newstr2[17] = "                ";
    char* newstrptr2= newstr2;
    char* temp1 = newstrptr1;
    char* temp2 = newstrptr2;
    uint8_t i=0;
    while (i <17){
        if (*message){
            *newstrptr1=*message;
            message++;
            newstrptr1++;
        }
        i++;
    }
    i=0;
    message--; //SKETCHY!
    while (i <17){
        if (*message){
            *newstrptr2=*message;
            message++;
            newstrptr2++;
        }
        i++;
    }
    lcd_print2(self, temp1, temp2);
}

void lcd_broadcast(char* message) {
    uint8_t i;
    for (i = 0; i < 3; i++)
        lcd_print(&lcd[i], message);
}
