#ifndef BAT_CAL_H
#define BAT_CAL_H

#define FILTER_SIZE 50   // number of samples to average (50 samples)

float voltage_to_soc(float voltage);
float filter_voltage(float new_reading);    //moving average

#endif