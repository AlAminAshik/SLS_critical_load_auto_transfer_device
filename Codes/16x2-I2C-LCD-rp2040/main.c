#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "lcd_i2c.h"

#define I2C_PORT i2c0
#define SDA_PIN 4
#define SCL_PIN 5
#define LCD_ADDR 0x27   // common default; try 0x3F if this doesn't work

int main() {
    stdio_init_all();

    i2c_init(I2C_PORT, 100 * 1000); // 100kHz — safer for these backpacks than 400kHz
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    lcd_init(I2C_PORT, LCD_ADDR);
    lcd_clear();

    while (true) {
        lcd_set_cursor(0, 0);
        lcd_print("Hello, RP2040!");
        lcd_set_cursor(0, 1);
        lcd_print("16x2 I2C LCD");
        sleep_ms(500);
        lcd_clear();
    }
}