#ifndef METEO_DATA_H
#define METEO_DATA_H

#include "FloatDeque.h"
#include <LiquidCrystal_PCF8574.h>

extern LiquidCrystal_PCF8574 lcd;
extern FloatDeque temp_1m, temp_10m, temp_2h, temp_1d, temp_12d;
extern FloatDeque hum_1m, hum_10m, hum_2h, hum_1d, hum_12d;
extern FloatDeque CO2_1m, CO2_10m, CO2_2h, CO2_1d, CO2_12d;
extern uint8_t index_1m, index_10m, index_2h, index_1d, index_12d;
extern float temp[12];

void printMeteoData();
void writeMeteoData(void *parameter);
void drawBarGraph(FloatDeque &data, uint8_t dataSize, uint8_t index, String name, String time);

#endif // METEO_DATA_H 