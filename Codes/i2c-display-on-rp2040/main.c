#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

#define I2C_PORT i2c0
#define SDA_PIN 4
#define SCL_PIN 5

int main() {
    stdio_init_all();

    i2c_init(I2C_PORT, 400 * 1000); // 400kHz I2C
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    ssd1306_init(I2C_PORT);

    ssd1306_clear();
    ssd1306_draw_string(0, 0, "Hello, RP2040!");
    ssd1306_draw_string(0, 2, "SSD1306 OLED");
    ssd1306_display();

    while (true) {
        sleep_ms(1000);
    }
}