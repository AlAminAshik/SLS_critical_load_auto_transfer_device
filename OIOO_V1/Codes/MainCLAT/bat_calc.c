#include "bat_calc.h"
#include <stdint.h>
#include "pico/stdlib.h"

#define SOC_TABLE_SIZE (sizeof(soc_table) / sizeof(soc_table[0]))

float voltage_to_soc(float voltage) {
    // Clamp to table bounds
    if (voltage >= soc_table[0].voltage) {
        return 100.0f;
    }
    if (voltage <= soc_table[SOC_TABLE_SIZE - 1].voltage) {
        return 0.0f;
    }

    // Find the two points voltage falls between, then linearly interpolate
    for (uint8_t i = 0; i < SOC_TABLE_SIZE - 1; i++) {
        float v_high = soc_table[i].voltage;
        float v_low  = soc_table[i + 1].voltage;

        if (voltage <= v_high && voltage >= v_low) {
            float soc_high = soc_table[i].soc;
            float soc_low  = soc_table[i + 1].soc;

            // Linear interpolation between the two bracketing points
            float ratio = (voltage - v_low) / (v_high - v_low);
            return soc_low + ratio * (soc_high - soc_low); //equation of line
        }
    }

    return 0.0f; // shouldn't reach here given the clamps above
}


static float voltage_buffer[FILTER_SIZE];
static uint8_t buffer_index = 0;
static bool buffer_filled = false;

float filter_voltage(float new_reading) {
    voltage_buffer[buffer_index] = new_reading;
    buffer_index = (buffer_index + 1) % FILTER_SIZE;
    if (buffer_index == 0) buffer_filled = true;

    uint8_t count = buffer_filled ? FILTER_SIZE : buffer_index;
    float sum = 0;
    for (uint8_t i = 0; i < count; i++) {
        sum += voltage_buffer[i];
    }
    return sum / count;
}