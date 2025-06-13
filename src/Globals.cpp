#include "Globals.h"
#include "Config.h"
#include "FloatDeque.h"
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_PCF8574.h>
#include "SparkFun_SCD4x_Arduino_Library.h"

// -------   Объект RTC и LCD   -------
RTC_DS3231 rtc;
LiquidCrystal_PCF8574 lcd(0x27);
SCD4x SCD40;

// -------   Флаги   -------
bool Clock_Settings_Done = false;
bool Graph_Settings_Done = false;
bool Settings_Done = false;
bool Graph_Reset = false;
bool Sleep = false;

// -------   Переменные энкодера   -------
volatile int ENC_Pos = 0;
volatile int ENC_LastPosition = 0;
long baseEncoder = ENC_Pos; 

// -------   Переменные для таймеров   -------
unsigned long lastTimeUpdate = 0;

// -------   Переменные для меню   -------
bool swichMode = 0;
int menuMode = 0;
int undermenuMode = 0;

// -------   Переменные, Класс и Деки для работы с метеоданными   ---------
// деки температури 
FloatDeque temp_1m(DEQUE_SIZE_1M);
FloatDeque temp_10m(DEQUE_SIZE_10M);
FloatDeque temp_2h(DEQUE_SIZE_2H);
FloatDeque temp_1d(DEQUE_SIZE_1D);
FloatDeque temp_12d(DEQUE_SIZE_12D);

// деки влажности 
FloatDeque hum_1m(DEQUE_SIZE_1M);
FloatDeque hum_10m(DEQUE_SIZE_10M);
FloatDeque hum_2h(DEQUE_SIZE_2H);
FloatDeque hum_1d(DEQUE_SIZE_1D);
FloatDeque hum_12d(DEQUE_SIZE_12D);

// деки СО2
FloatDeque CO2_1m(DEQUE_SIZE_1M);
FloatDeque CO2_10m(DEQUE_SIZE_10M);
FloatDeque CO2_2h(DEQUE_SIZE_2H);
FloatDeque CO2_1d(DEQUE_SIZE_1D);
FloatDeque CO2_12d(DEQUE_SIZE_12D);

uint8_t index_1m = 0, index_10m = 0, index_2h = 0, index_1d = 0, index_12d = 0, index_temp = 0;
float temp[12] = {0}; 