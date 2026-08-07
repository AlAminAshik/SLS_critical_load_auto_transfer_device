#include "lcd_i2c.h"
#include "pico/stdlib.h"
#include <stdio.h>

static i2c_inst_t *lcd_i2c;
static uint8_t lcd_addr;
static uint8_t backlight = 0x08; // backlight ON bit (P3)

// LCD command bits
#define LCD_RS 0x01
#define LCD_RW 0x02
#define LCD_EN 0x04

static void i2c_write_byte(uint8_t data) {
    uint8_t buf = data | backlight;
    i2c_write_blocking(lcd_i2c, lcd_addr, &buf, 1, false);
}

static void pulse_enable(uint8_t data) {
    i2c_write_byte(data | LCD_EN);
    sleep_us(1);
    i2c_write_byte(data & ~LCD_EN);
    sleep_us(50);
}

static void lcd_write4(uint8_t nibble, uint8_t mode) {
    uint8_t data = (nibble & 0xF0) | mode;
    i2c_write_byte(data);
    pulse_enable(data);
}

static void lcd_send(uint8_t value, uint8_t mode) {
    lcd_write4(value & 0xF0, mode);         // high nibble
    lcd_write4((value << 4) & 0xF0, mode);  // low nibble
}

static void lcd_cmd(uint8_t cmd) {
    lcd_send(cmd, 0);
}

static void lcd_data(uint8_t data) {
    lcd_send(data, LCD_RS);
}

void lcd_init(i2c_inst_t *i2c, uint8_t addr) {
    lcd_i2c = i2c;
    lcd_addr = addr;

    sleep_ms(50); // wait for LCD power-on

    // Init sequence for 4-bit mode (standard HD44780 procedure)
    lcd_write4(0x30, 0);
    sleep_ms(5);
    lcd_write4(0x30, 0);
    sleep_us(150);
    lcd_write4(0x30, 0);
    lcd_write4(0x20, 0); // set to 4-bit mode

    lcd_cmd(0x28); // function set: 4-bit, 2 line, 5x8 font
    lcd_cmd(0x0C); // display ON, cursor OFF, blink OFF
    lcd_cmd(0x06); // entry mode: increment cursor, no shift
    lcd_clear();
}

void lcd_clear(void) {
    lcd_cmd(0x01);
    sleep_ms(2); // clear command needs extra time
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    static const uint8_t row_offsets[] = {0x00, 0x40};
    if (row > 1) row = 1;
    lcd_cmd(0x80 | (col + row_offsets[row]));
}

void lcd_print(const char *str) {
    while (*str) {
        lcd_data(*str++);
    }
}

void lcd_print_num(float num, int decimal_places) {
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%.*f", decimal_places, num);
    lcd_print(buffer);
}

void lcd_print_num_padded(float value, uint8_t decimals, uint8_t width) {
    char fmt[8];
    char buf[16];
    snprintf(fmt, sizeof(fmt), "%%%u.%uf", width, decimals);
    snprintf(buf, sizeof(buf), fmt, value);
    lcd_print(buf);
}