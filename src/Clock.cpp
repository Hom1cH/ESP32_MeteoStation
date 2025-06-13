#include "Clock.h"
#include "Config.h"
#include "Fonts.h"
#include <RTClib.h>

extern RTC_DS3231 rtc;
extern bool Graph_Settings_Done;

void printNumber(uint8_t number, uint8_t col) {
   switch (number) {
    case 0:
        lcd.setCursor(col,0);
        lcd.write(2);
        lcd.write(1);
        lcd.write(3);
        lcd.setCursor(col,1);
        lcd.write(0);
        lcd.print(" ");
        lcd.write(0);
        lcd.setCursor(col,2);
        lcd.write(6);
        lcd.write(1);
        lcd.write(7);
        break;
    case 1:
        lcd.setCursor(col,0);
        lcd.write(4);
        lcd.write(0);
        lcd.print(" ");
        lcd.setCursor(col,1);
        lcd.print(" ");
        lcd.write(0);
        lcd.print(" ");
        lcd.setCursor(col,2);
        lcd.write(1);
        lcd.write(1);
        lcd.write(1);
        break;
    case 2:
        lcd.setCursor(col,0);
        lcd.write(4);
        lcd.write(1);
        lcd.write(3);
        lcd.setCursor(col,1);
        lcd.write(2);
        lcd.write(1);
        lcd.write(1);
        lcd.setCursor(col,2);
        lcd.write(1);
        lcd.write(1);
        lcd.write(1);
        break;
    case 3:
        lcd.setCursor(col,0);
        lcd.write(4);
        lcd.write(1);
        lcd.write(3);
        lcd.setCursor(col,1);
        lcd.print(" ");
        lcd.write(1);
        lcd.write(0);
        lcd.setCursor(col,2);
        lcd.write(6);
        lcd.write(1);
        lcd.write(7);
        break;
    case 4:
        lcd.setCursor(col,0);
        lcd.write(0);
        lcd.print(" ");
        lcd.write(0);
        lcd.setCursor(col,1);
        lcd.write(1);
        lcd.write(1);
        lcd.write(0);
        lcd.setCursor(col,2);
        lcd.print(" ");
        lcd.print(" ");
        lcd.write(1);
        break;
    case 5:
        lcd.setCursor(col,0);
        lcd.write(0);
        lcd.write(1);
        lcd.write(1);
        lcd.setCursor(col,1);
        lcd.write(1);
        lcd.write(1);
        lcd.write(0);
        lcd.setCursor(col,2);
        lcd.write(6);
        lcd.write(1);
        lcd.write(7);
        break;
    case 6:
        lcd.setCursor(col,0);
        lcd.write(2);
        lcd.write(1);
        lcd.write(5);
        lcd.setCursor(col,1);
        lcd.write(0);
        lcd.write(1);
        lcd.write(3);
        lcd.setCursor(col,2);
        lcd.write(6);
        lcd.write(1);
        lcd.write(7);
        break;
    case 7:
        lcd.setCursor(col,0);
        lcd.write(1);
        lcd.write(1);
        lcd.write(0);
        lcd.setCursor(col,1);
        lcd.print(" ");
        lcd.write(2);
        lcd.write(7);
        lcd.setCursor(col,2);
        lcd.print(" ");
        lcd.write(1);
        lcd.print(" ");
        break;
    case 8:
        lcd.setCursor(col,0);
        lcd.write(2);
        lcd.write(1);
        lcd.write(3);
        lcd.setCursor(col,1);
        lcd.write(0);
        lcd.write(1);
        lcd.write(0);
        lcd.setCursor(col,2);
        lcd.write(6);
        lcd.write(1);
        lcd.write(7);
        break;
    case 9:
        lcd.setCursor(col,0);
        lcd.write(2);
        lcd.write(1);
        lcd.write(3);
        lcd.setCursor(col,1);
        lcd.write(6);
        lcd.write(1);
        lcd.write(0);
        lcd.setCursor(col,2);
        lcd.write(6);
        lcd.write(1);
        lcd.write(7);
        break;
    default:
        break;
   }
}

void printClock() {
    if(Clock_Settings_Done == false) {
        for (uint8_t i = 0; i < 8; i++) {
            lcd.createChar(i, (uint8_t*)ClockChars[i]);
        }
        Graph_Settings_Done = false;
        Clock_Settings_Done = true;
    }
    
    if(millis() - lastTimeUpdate >= 1000) {
        lastTimeUpdate = millis();
        DateTime now = rtc.now();
        if(now.hour() < 10) {
            printNumber(0,0);
            printNumber(now.hour(),4);
        } else {
            printNumber((now.hour()-(now.hour() % 10))/10,0);
            printNumber(now.hour() % 10,4);
        }
        lcd.setCursor(7,1);
        lcd.print(":");
        if(now.minute() < 10) {
            printNumber(0,8);
            printNumber(now.minute(),12);
        } else {
            printNumber((now.minute()-(now.minute() % 10))/10,8);
            printNumber(now.minute() % 10,12);
        }
        
        lcd.setCursor(0, 3);
        lcd.print("Date: ");
        const char* daysOfWeek[] = {"Sun ", "Mon ", "Tue ", "Wed ", "Thu ", "Fri ", "Sat "};
        lcd.print(daysOfWeek[now.dayOfTheWeek()]);
        lcd.print(now.day() < 10 ? "0" : ""); lcd.print(now.day());
        lcd.print(".");
        lcd.print(now.month() < 10 ? "0" : ""); lcd.print(now.month());
        lcd.print(".");
        lcd.print(now.year());
    }
}

void printNewTime(int params[]) {
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  if (params[3] < 10) lcd.print("0");
  lcd.print(params[3]);
  lcd.print(":");
  if (params[4] < 10) lcd.print("0");
  lcd.print(params[4]);
  lcd.print(":");
  if (params[5] < 10) lcd.print("0");
  lcd.print(params[5]);
}

void printNewDate(int params[]) {
  lcd.setCursor(0, 2);
  lcd.print("Date: ");
  if (params[2] < 10) lcd.print("0");
  lcd.print(params[2]);
  lcd.print(".");
  if (params[1] < 10) lcd.print("0");
  lcd.print(params[1]);
  lcd.print(".");
  lcd.print(params[0]);
}

void printCursor(uint8_t paramIndex) {
  switch (paramIndex) {
    case 0: lcd.setCursor(7, 1); lcd.print("^"); break;
    case 1: lcd.setCursor(10, 1); lcd.print("^"); break;
    case 2: lcd.setCursor(13, 1); lcd.print("^"); break;
    case 3: lcd.setCursor(7, 3); lcd.print("^"); break;
    case 4: lcd.setCursor(10, 3); lcd.print("^"); break;
    case 5: lcd.setCursor(13, 3); lcd.print("^^"); break;
  }
}

void setTimeManual() {
  bool Time_Setting_Done = false;
  int params[6] = {2000, 1, 1, 0, 0, 0};
  uint8_t paramIndex = 0;
  long baseEncoder = 0; // Используем локальную переменную

  while (digitalRead(BTN_PIN) == LOW) {
    delay(10);
  }

  lcd.clear();
  while (!Time_Setting_Done) {
    int step = (0 - baseEncoder) / 4; // Упрощенная логика

    if (step != 0) {
      switch (paramIndex) {
        case 0: { 
          int v = params[3] + step; 
          params[3] = constrain(v, 0, 23); 
        } break;
        case 1: { 
          int v = params[4] + step; 
          params[4] = constrain(v, 0, 59); 
        } break;
        case 2: { 
          int v = params[5] + step; 
          params[5] = constrain(v, 0, 59); 
        } break;
        case 3: { 
          int v = params[2] + step; 
          params[2] = constrain(v, 1, 31); 
        } break;
        case 4: { 
          int v = params[1] + step; 
          params[1] = constrain(v, 1, 12); 
        } break;
        case 5: { 
          int v = params[0] + step; 
          params[0] = constrain(v, 2000, 2099); 
        } break;
      }
      baseEncoder += step * 4;
    }

    printNewTime(params);
    printNewDate(params);
    printCursor(paramIndex);

    if (digitalRead(BTN_PIN) == LOW) {
      delay(200);
      paramIndex++;
      lcd.clear();
      if (paramIndex >= 6) {
        while (digitalRead(BTN_PIN) == LOW) delay(10);
        delay(300);
        rtc.adjust(DateTime(params[0], params[1], params[2], params[3], params[4], params[5]));
        Time_Setting_Done = true;
      } else {
        baseEncoder = 0;
      }
    }
    delay(10);
  }
} 