#include "Menu.h"
#include "Config.h"
#include "Clock.h"
#include "MeteoData.h"
#include "Settings.h"
#include <LiquidCrystal_PCF8574.h>
#include <RTClib.h>

extern LiquidCrystal_PCF8574 lcd;
extern RTC_DS3231 rtc;
extern volatile int ENC_Pos;
extern volatile int ENC_LastPosition;
extern long baseEncoder;
extern bool Settings_Done;
extern bool Sleep;
extern int menuMode;
extern int undermenuMode;
extern bool swichMode;
extern bool Graph_Reset;

// Глобальные переменные для меню
MenuManager menuManager;
volatile bool buttonPressed = false;
volatile unsigned long lastButtonInterrupt = 0;

void IRAM_ATTR buttonInterrupt() {
  unsigned long currentTime = millis();
  
  // Простой debounce в прерывании (200ms)
  if (currentTime - lastButtonInterrupt > 200) {
    buttonPressed = true;
    lastButtonInterrupt = currentTime;
  }
}

MenuManager::MenuManager() {
  // Часы
  menu[0][0] = { printClock };

  // Текущие метеоданные
  menu[1][0] = { printMeteoData };

  // Температура
  menu[2][0] = { []() { drawBarGraph(temp_1m, 12, index_1m, "Temp", "1min"); }};
  menu[2][1] = { []() { drawBarGraph(temp_10m, 10, index_10m, "Temp", "10min"); }};
  menu[2][2] = { []() { drawBarGraph(temp_2h, 12, index_2h, "Temp", "2h"); }};
  menu[2][3] = { []() { drawBarGraph(temp_1d, 12, index_1d, "Temp", "1d"); }};
  menu[2][4] = { []() { drawBarGraph(temp_12d, 12, index_12d, "Temp", "12d"); }};

  // Влажность
  menu[3][0] = { []() { drawBarGraph(hum_1m, 12, index_1m, "Hum", "1min"); }};
  menu[3][1] = { []() { drawBarGraph(hum_10m, 10, index_10m, "Hum", "10min"); }};
  menu[3][2] = { []() { drawBarGraph(hum_2h, 12, index_2h, "Hum", "2h"); }};
  menu[3][3] = { []() { drawBarGraph(hum_1d, 12, index_1d, "Hum", "1d"); }};
  menu[3][4] = { []() { drawBarGraph(hum_12d, 12, index_12d, "Hum", "12d"); }};

  // CO2
  menu[4][0] = { []() { drawBarGraph(CO2_1m, 12, index_1m, "CO2", "1min"); }};
  menu[4][1] = { []() { drawBarGraph(CO2_10m, 10, index_10m, "CO2", "10min"); }};
  menu[4][2] = { []() { drawBarGraph(CO2_2h, 12, index_2h, "CO2", "2h"); }};
  menu[4][3] = { []() { drawBarGraph(CO2_1d, 12, index_1d, "CO2", "1d"); }};
  menu[4][4] = { []() { drawBarGraph(CO2_12d, 12, index_12d, "CO2", "12d"); }};

  //
  menu[5][0] = { []() {
    lcd.setCursor(0,0);
    lcd.print("Comp DATA");
    lcd.setCursor(0,1);
    lcd.print("Comming soon..."); 
  }};
  
  //
  menu[6][0] = { []() { 
    lcd.setCursor(0,0);
    lcd.print("Test 1"); 
  }};
  //
  menu[7][0] = { []() { 
    lcd.setCursor(0,0);
    lcd.print("Test 2"); 
  }};
  
  //
  menu[8][0] = { []() { 
    lcd.setCursor(0,0);
    lcd.print("Settings"); 
  }};
  menu[8][1] = { []() { settings(); }};
}

int MenuManager::undermenuSize(int row) {
  if (row < 0 || row >= ROWS) return 0;
  
  int count = 0;
  for (int i = 0; i < COLS; i++) {
    if (menu[row][i].functionPtr) {
      count++;
    } else {
      break;
    }
  }
  return count;
}

void MenuManager::updateEncoder() {
  int ENC_First = digitalRead(ENC_F_PIN);
  int ENC_Last = digitalRead(ENC_L_PIN);
  int ENC_CurentPosition = (ENC_First << 1) | ENC_Last;
  int ENC_Sum = (ENC_LastPosition << 2) | ENC_CurentPosition;

  if (ENC_Sum == 0b1101 || ENC_Sum == 0b0100 || ENC_Sum == 0b0010 || ENC_Sum == 0b1011)
    ENC_Pos++;
  if (ENC_Sum == 0b1110 || ENC_Sum == 0b0111 || ENC_Sum == 0b0001 || ENC_Sum == 0b1000)
    ENC_Pos--;
  ENC_LastPosition = ENC_CurentPosition;
}

void MenuManager::modeSwicher() {
  if(undermenuSize(menuMode) == 1) {
    return;
  } else {
    swichMode = !swichMode;
    
    // Сброс подменю при переключении режимов
    if (!swichMode) {
      undermenuMode = 0;
    }
    if(swichMode == 0) {
      undermenuMode = 0;
    }
    if(swichMode == 1) {
      undermenuMode = 1;
    }
  }
}

void MenuManager::Swicher() {
  int step = (ENC_Pos - baseEncoder) / 4;
  if (step != 0) {
    
    if(swichMode == 0) {
      // Переключение между основными меню
      menuMode += step;
      
      // Обработка границ
      if (menuMode >= ROWS) {
        menuMode = 0;
      }
      if (menuMode < 0) {
        menuMode = ROWS - 1;
      }
      
      undermenuMode = 0;  // Сброс подменю
      Graph_Reset = false;
      lcd.clear();
    } else {
      // Переключение в подменю
      int maxSubmenu = undermenuSize(menuMode) - 1;
      
      if (maxSubmenu >= 0) {
        undermenuMode += step;
        
        // Правильная обработка границ подменю
        if (undermenuMode > maxSubmenu) {
          undermenuMode = 0;
        }
        if (undermenuMode < 0) {
          undermenuMode = maxSubmenu;
        }
        
        Graph_Reset = false;
        lcd.clear();
      }
    }
    
    // Обновляем базовую позицию энкодера
    baseEncoder += step * 4;
  }
}

void MenuManager::showCurrent() {
  // Добавляем защиту от слишком частых обновлений
  static unsigned long lastMenuUpdate = 0;
  unsigned long currentTime = millis();
  
  // Обновляем меню не чаще чем раз в 50мс
  if (currentTime - lastMenuUpdate < 50) {
    return;
  }
  
  Swicher();
  
  // Проверяем валидность индексов меню
  if (menuMode < 0 || menuMode >= ROWS) {
    Serial.println("ERROR: Invalid menuMode");
    menuMode = 0;
  }
  
  if (undermenuMode < 0 || undermenuMode >= COLS) {
    Serial.println("ERROR: Invalid undermenuMode");
    undermenuMode = 0;
  }
  
  MenuSettings& item = menu[menuMode][undermenuMode];
  if (item.functionPtr) {
    item.functionPtr();
  } else {
    Serial.println("ERROR: NOT_A_FUNCTION");
  }
  
  lastMenuUpdate = currentTime;
} 