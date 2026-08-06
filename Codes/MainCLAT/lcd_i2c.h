#ifndef LCD_I2C_H
#define LCD_I2C_H

#include "hardware/i2c.h"

void lcd_init(i2c_inst_t *i2c, uint8_t addr);
void lcd_clear(void);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_print(const char *str);
void lcd_print_num(float num);

#endif