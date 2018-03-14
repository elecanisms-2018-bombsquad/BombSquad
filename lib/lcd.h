#ifndef _LCD_H_
#define _LCD_H_

#include <stdint.h>
#include "i2c_reg.h"

void init_lcd(uint8_t initiator);

typedef struct {
    uint8_t addr_write;
    uint8_t addr_read;

    uint8_t io_write_val;
    uint8_t display_control;
    uint8_t display_mode;
} _LCD;

extern _LCD lcd[3];

void lcd_init(_LCD *self, uint8_t addr, char vendor);
void lcd_stop(_LCD *self);

void lcd_putc(_LCD *self, char c);
void lcd_clear(_LCD *self);

void lcd_display(_LCD *self, uint8_t on);
void lcd_goto(_LCD *self, uint8_t line, uint8_t col);
void lcd_cursor(_LCD *self, uint8_t cur);

void lcd_print1(_LCD *self, char *str);
void lcd_print2(_LCD *self, char* line1, char* line2);
void lcd_print(_LCD *self, char* message);
void lcd_broadcast(char* message);
char * itoa (int value, char *result, int base);
#endif
