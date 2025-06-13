#include "Settings.h"
#include "Config.h"
#include "Clock.h"
#include "Globals.h"
#include <LiquidCrystal_PCF8574.h>
#include <RTClib.h>

extern LiquidCrystal_PCF8574 lcd;
extern RTC_DS3231 rtc;
extern volatile int ENC_Pos;
extern long baseEncoder;
extern bool Settings_Done;
extern bool Sleep;
extern int undermenuMode;
extern bool Graph_Reset;

void settings() {
  Settings_Done = false;
  undermenuMode = 6;  // начинаем с первого пункта
  
  while (Settings_Done != true)  // цикл работает, пока не нажата кнопка
  {
    // Переключение между подменю
    int step = (ENC_Pos - baseEncoder) / 4;
    if (step != 0) {
      undermenuMode += step;
      baseEncoder += step * 4;
      if (undermenuMode > 6) undermenuMode = 1;
      if (undermenuMode < 1) undermenuMode = 6;
      Graph_Reset = false;
      lcd.clear();
    }

    lcd.setCursor(0, 0);
    lcd.print("Settings:");
    lcd.setCursor(0, 1);
    lcd.print("Set time");
    lcd.setCursor(0, 2);
    lcd.print("Set_3");
    lcd.setCursor(0, 3);
    lcd.print("Set_5");
    lcd.setCursor(15, 1);
    lcd.print("Sleep");
    lcd.setCursor(15, 2);
    lcd.print("Set_4");
    lcd.setCursor(16, 3);
    lcd.print("EXIT");
    
    switch (undermenuMode) {
      case 1:
        lcd.setCursor(9, 1);
        lcd.print("<");
        if(digitalRead(BTN_PIN) == LOW) {
          delay(100);
          lcd.clear();
          setTimeManual();
        }
        break;
      case 2:
        lcd.setCursor(10, 1);
        lcd.print(">");
        if(digitalRead(BTN_PIN) == LOW) {
          lcd.clear();
          lcd.setBacklight(false);
          Sleep = true;
          while (Sleep == true) {
            delay(500);
            if(digitalRead(BTN_PIN) == LOW) {
              Sleep = false;
            } 
          }
          lcd.setBacklight(true);
          delay(500);
          Settings_Done = true;
        }
        break;
      case 3:
        lcd.setCursor(9, 2);
        lcd.print("<");
        break;
      case 4:
        lcd.setCursor(10, 2);
        lcd.print(">");
        break;
      case 5:
        lcd.setCursor(9, 3);
        lcd.print("<");
        break;
      case 6:
        lcd.setCursor(10, 3);
        lcd.print(">");
        if(digitalRead(BTN_PIN) == LOW) {
          delay(100);
          lcd.clear();
          Settings_Done = true;
        }
        break;
      default:
        break;
    }
    delay(150);
  }
  extern int menuMode;
  extern bool swichMode;
  menuMode = 0;
  undermenuMode = 0;
  swichMode = 0;
} 