#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include <stdio.h>
#include "lcd_i2c.h"
#include "bat_calc.h"

#define I2C_PORT i2c0
#define SDA_PIN 4
#define SCL_PIN 5
#define LCD_ADDR 0x27   // common default; try 0x3F if this doesn't work

#define ADC_PIN 26        // GP26 = ADC0. Also available: GP27 (ADC1), GP28 (ADC2)
#define ADC_CHANNEL 0     // 0 for GP26, 1 for GP27, 2 for GP28


int main() {
    stdio_init_all();

    // Initialize I2C display
    i2c_init(I2C_PORT, 100 * 1000); // 100kHz — safer for these backpacks than 400kHz
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
    lcd_init(I2C_PORT, LCD_ADDR);
    lcd_clear();

    // Initialize ADC
    adc_init();                 // Initialize the ADC hardware
    adc_gpio_init(ADC_PIN);     // Initialize the GPIO pin for ADC input
    adc_select_input(ADC_CHANNEL);  // Select the ADC channel corresponding to the pin
    // Wait a moment for USB serial to connect (helps catch early prints)
    sleep_ms(2000);
    
    const float conversion_factor = 3.30f / (1 << 12);  // 12-bit ADC, 3.3V reference (1 << 12 is just a slightly more "hardware-flavored" way of writing 4096 (2^12)))


    while (true) {
        uint16_t raw = adc_read();              // raw value: 0–4095
        float voltage = raw * conversion_factor * 20; // Convert to actual voltage at the battery terminals  
        float filtered_voltage = filter_voltage(voltage); // Apply moving average filter
        float soc = voltage_to_soc(filtered_voltage);     // Convert voltage to SOC percentage

        printf("Raw: %4d | Voltage: %.3f V | Filtered: %.3f V | SOC: %.2f %%\n", raw, voltage, filtered_voltage, soc);

        lcd_set_cursor(0, 0);
        lcd_print("Voltage: "); lcd_print_num(filtered_voltage); lcd_print(" V");
        lcd_set_cursor(0, 1);
        lcd_print("SOC: "); lcd_print_num(soc); lcd_print(" %");
        sleep_ms(300);
        // lcd_clear();
    }
}