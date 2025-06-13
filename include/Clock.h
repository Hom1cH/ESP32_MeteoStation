#ifndef CLOCK_H
#define CLOCK_H

#include <LiquidCrystal_PCF8574.h>

extern LiquidCrystal_PCF8574 lcd;
extern bool Clock_Settings_Done;
extern unsigned long lastTimeUpdate;

void printNumber(uint8_t number, uint8_t col);
void printClock();
void printNewTime(int params[]);
void printNewDate(int params[]);
void printCursor(uint8_t paramIndex);
void setTimeManual();

#endif // CLOCK_H 