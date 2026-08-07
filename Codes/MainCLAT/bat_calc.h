#ifndef BAT_CAL_H
#define BAT_CAL_H

#define FILTER_SIZE 50   // number of samples to average (50 samples)

// LiFePO4 48V pack: voltage -> SOC% lookup table (rest/no-load values)
// Sorted descending by voltage for interpolation logic below.
typedef struct {
    float voltage;
    float soc;
} soc_point_t;

static const soc_point_t soc_table[] = {
    {58.4f, 100.0f},  // charging voltage (transient, not true rest SOC)
    {54.4f, 100.0f},  // rest voltage at full charge
    {53.6f, 90.0f},
    {53.1f, 80.0f},
    {52.8f, 70.0f},
    {52.3f, 60.0f},
    {52.2f, 50.0f},
    {52.0f, 40.0f},
    {51.5f, 30.0f},
    {51.2f, 20.0f},
    {48.0f, 10.0f},
    {40.0f, 0.0f},
};

#define RELAY_OFF_THRESHOLD 80.0f   // 80% SOC — turn relay OFF at or below this
#define RELAY_ON_THRESHOLD  50.0f   // 50% SOC — only turn relay back ON above this


float voltage_to_soc(float voltage);
float filter_voltage(float new_reading);    //moving average
void update_relay(float soc);  // Update relay state based on SOC

#endif