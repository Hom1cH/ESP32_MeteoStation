#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_PCF8574.h>
#include "SparkFun_SCD4x_Arduino_Library.h"
#include "FloatDeque.h"
#include "Config.h"

// -------   Объект RTC и LCD   -------
extern RTC_DS3231 rtc;
extern LiquidCrystal_PCF8574 lcd;
extern SCD4x SCD40;

// -------   Флаги   -------
extern bool Clock_Settings_Done;
extern bool Graph_Settings_Done;
extern bool Settings_Done;
extern bool Graph_Reset;
extern bool Sleep;

// -------   Переменные энкодера   -------
extern volatile int ENC_Pos;
extern volatile int ENC_LastPosition;
extern long baseEncoder;

// -------   Переменные для таймеров   -------
extern unsigned long lastTimeUpdate;

// -------   Переменные для меню   -------
extern bool swichMode;
extern int menuMode;
extern int undermenuMode;

// -------   Переменные, Класс и Деки для работы с метеоданными   ---------
// деки температури 
extern FloatDeque temp_1m;
extern FloatDeque temp_10m;
extern FloatDeque temp_2h;
extern FloatDeque temp_1d;
extern FloatDeque temp_12d;

// деки влажности 
extern FloatDeque hum_1m;
extern FloatDeque hum_10m;
extern FloatDeque hum_2h;
extern FloatDeque hum_1d;
extern FloatDeque hum_12d;

// деки СО2
extern FloatDeque CO2_1m;
extern FloatDeque CO2_10m;
extern FloatDeque CO2_2h;
extern FloatDeque CO2_1d;
extern FloatDeque CO2_12d;

extern uint8_t index_1m, index_10m, index_2h, index_1d, index_12d, index_temp;
extern float temp[12];

#endif // GLOBALS_H 