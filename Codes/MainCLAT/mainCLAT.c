#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include <stdio.h>
#include "lcd_i2c.h"
#include "bat_calc.h"
#include "pico/time.h"
#include "hardware/watchdog.h"

int PAGE_SWITCH_INTERVAL_MS = 5000;   // 5 seconds per page
static uint8_t current_page = 0;
static absolute_time_t last_page_switch;

#define I2C_PORT i2c0
#define SDA_PIN 4
#define SCL_PIN 5
#define LCD_ADDR 0x27   // common default; try 0x3F if this doesn't work

#define ADC_PIN 26        // GP26 = ADC0. Also available: GP27 (ADC1), GP28 (ADC2)
#define ADC_CHANNEL 0     // 0 for GP26, 1 for GP27, 2 for GP28

// GPIO control
#define BUZ_PIN 14
#define RELAY_PIN 8
#define GRID_IN_PIN 15

float soc = 0.0f;
float resistor_Ratio = 30.5f;

static bool relayState = false;  // initially OFF
static bool buzzerState = false; // initially OFF

void show_splash_screen(void) {
    lcd_clear();
    lcd_set_cursor(4, 0);
    lcd_print("SLS CLAT");
    lcd_set_cursor(0, 1);
    lcd_print("Supreme Lithium");
    sleep_ms(3000); 
    lcd_clear();
}

static void display_page1(float filtered_voltage, float soc, bool relayState) {
    // --- Top row: voltage + SOC
    lcd_set_cursor(0, 0);
    lcd_print("V:");
    lcd_print_num_padded(filtered_voltage, 1, 4);   // e.g. "52.2"
    lcd_print("V");
    lcd_set_cursor(8, 0);
    lcd_print("SOC:");
    lcd_print_num_padded(soc, 0, 3);                // e.g. " 76" or "100"
    lcd_print("%");

    // --- Bottom row: CL1 / CL2 relay status
    lcd_set_cursor(0, 1);
    lcd_print("CL1: ON");
    lcd_set_cursor(8, 1);
    lcd_print("CL2:");
    lcd_print(relayState ? "OFF " : " ON");

    PAGE_SWITCH_INTERVAL_MS = 5000;  // 5 seconds for page 1
}

static void display_page2(bool grid_ok) {
    lcd_set_cursor(3, 0);
    lcd_print("Mains ");
    lcd_print(grid_ok ? "OFF" : "ON ");   // GRID_IN_PIN low = mains ON, per your spec

    lcd_set_cursor(0, 1);
    lcd_print("OFF:");
    lcd_print_num(RELAY_ON_THRESHOLD, 1);
    // lcd_print("V");
    lcd_set_cursor(9, 1);
    lcd_print("ON:");
    lcd_print_num(RELAY_OFF_THRESHOLD, 1);
    // lcd_print("V");

    PAGE_SWITCH_INTERVAL_MS = 2000;  // 2 seconds for page 2
}

void soundBuzzer(int duration_ms) {
    gpio_put(BUZ_PIN, 1); // turn buzzer ON
    sleep_ms(duration_ms);
    gpio_put(BUZ_PIN, 0); // turn buzzer OFF
}

void update_relay(float value) {
    if (!relayState && value <= RELAY_ON_THRESHOLD) {
        relayState = true;
        soundBuzzer(300); // sound buzzer for 0.3 seconds when relay turns ON
        gpio_put(RELAY_PIN, 1); // turn relay ON
    } else if (relayState && value >= RELAY_OFF_THRESHOLD) {
        relayState = false;
        soundBuzzer(300); // sound buzzer for 0.3 seconds when relay turns OFF
        gpio_put(RELAY_PIN, 0); // turn relay OFF
    }
// Otherwise: stay in current state — this is the hysteresis "dead zone"
}

int main() {
    stdio_init_all();
    watchdog_enable(4000, 1); // Enable watchdog with a 4-second timeout and pause on debug

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
    // // Wait a moment for USB serial to connect (helps catch early prints)
    // sleep_ms(2000);
    
    // GPIO setup
    gpio_init(BUZ_PIN);
    gpio_set_dir(BUZ_PIN, GPIO_OUT);
    gpio_init(RELAY_PIN);
    gpio_set_dir(RELAY_PIN, GPIO_OUT);
    gpio_init(GRID_IN_PIN);
    gpio_set_dir(GRID_IN_PIN, GPIO_IN);
    gpio_pull_up(GRID_IN_PIN);  // Enable internal pull-up resistor

    show_splash_screen();

    const float conversion_factor = 3.30f / (1 << 12);  // 12-bit ADC, 3.3V reference (1 << 12 is just a slightly more "hardware-flavored" way of writing 4096 (2^12)))
    
    last_page_switch = get_absolute_time();

    while (true) {
        uint16_t raw = adc_read();              // raw value: 0–4095
        float voltage = raw * conversion_factor * resistor_Ratio; // Convert to actual voltage at the battery terminals  
        float filtered_voltage = filter_voltage(voltage); // Apply moving average filter
        soc = voltage_to_soc(filtered_voltage);     // Convert voltage to SOC percentage

        printf("Raw: %4d | Voltage: %.3f V | Filtered: %.3f V | SOC: %.2f %%\n", raw, voltage, filtered_voltage, soc);

        bool grid_ok = gpio_get(GRID_IN_PIN);
        if (!grid_ok) {
            // GRID_IN_PIN low: bypass all SOC-based hysteresis logic, force relay off
            if (relayState) {
                relayState = false;
                soundBuzzer(1000);
                gpio_put(RELAY_PIN, 0);
            }
        } else {
            update_relay(filtered_voltage);  // only update relay state based on voltage if grid is OK
        }

        if (absolute_time_diff_us(last_page_switch, get_absolute_time()) >= PAGE_SWITCH_INTERVAL_MS * 1000) {
            current_page = (current_page + 1) % 2;
            last_page_switch = get_absolute_time();
            lcd_clear();   // only clear on page change, not every 100ms — avoids flicker
        }

        if (current_page == 0) {
            display_page1(filtered_voltage, soc, relayState);
        } else {
            display_page2(grid_ok);
        }
        
        sleep_ms(100);
        watchdog_update();
    }
}