#ifndef MENU_H
#define MENU_H

#include <Arduino.h>

// Структура одного пункта меню
struct MenuSettings {
  void (*functionPtr)();
};

// Менеджер меню
class MenuManager {
  public:
    static const int ROWS = 9;
    static const int COLS = 5;
    
    int menuMode = 0;
    int undermenuMode = 0;
    bool swichMode = 0;

    MenuSettings menu[ROWS][COLS];
  
    MenuManager();
    int undermenuSize(int row);
    void updateEncoder();
    void modeSwicher();
    void Swicher();
    void showCurrent();
};

extern MenuManager menuManager;
extern volatile bool buttonPressed;
extern volatile unsigned long lastButtonInterrupt;

void IRAM_ATTR buttonInterrupt();

#endif // MENU_H 