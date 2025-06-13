// --------  Библиотеки   --------
#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_PCF8574.h>
#include <SparkFun_SCD4x_Arduino_Library.h>

// --------  Наши заголовочные файлы   --------
#include "Config.h"
#include "Fonts.h"
#include "FloatDeque.h"
#include "Clock.h"
#include "MeteoData.h"
#include "Menu.h"
#include "Globals.h"

// -------   Setup и Loop   -------
void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(ENC_F_PIN, INPUT_PULLUP);
  pinMode(ENC_L_PIN, INPUT_PULLUP);

  if (! SCD40.begin(Wire)) {
    lcd.clear();
    lcd.print("SCD4x not found");
    while (1);
  }
  SCD40.startPeriodicMeasurement();

  attachInterrupt(digitalPinToInterrupt(ENC_F_PIN), [](){menuManager.updateEncoder();}, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_L_PIN), [](){menuManager.updateEncoder();}, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), buttonInterrupt, FALLING);  // Добавляем прерывание для кнопки

  uint8_t a = digitalRead(ENC_F_PIN);
  uint8_t b = digitalRead(ENC_L_PIN);
  ENC_LastPosition = (a << 1) | b;

  if (!rtc.begin()) {
    Serial.print("RTC not found");
    while (1);
  }

  DateTime now = rtc.now();

  Wire1.begin(16, 17, 100000);

  lcd.begin(20, 4, Wire1);
  lcd.setBacklight(true);

  // Если время сброшено (по умолчанию 2000 год) — запускаем ручную установку
  if (now.year() < 2024) {
    setTimeManual();
  }

  delay(1000);
  lcd.print(".");
  delay(1000);
  lcd.print(".");
  delay(1000);
  lcd.print(".");
  delay(1000);
  lcd.clear();
  lcd.print("Settings done!");
  delay(2000);
  lcd.clear();
  xTaskCreatePinnedToCore(writeMeteoData,"writeMeteoData",2048,NULL,1,NULL,0);
}

void loop() {
  // Обработка кнопки через прерывание
  if (buttonPressed) {
    menuManager.modeSwicher();
    buttonPressed = false;  // Сбрасываем флаг
  }
  
  menuManager.showCurrent();
  delay(100);
} 