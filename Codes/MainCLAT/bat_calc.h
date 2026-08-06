#ifndef BAT_CAL_H
#define BAT_CAL_H

#define FILTER_SIZE 20   // number of samples to average (20 samples @ 100Hz = 200ms window)

float voltage_to_soc(float voltage);
float filter_voltage(float new_reading);    //moving average

#endif