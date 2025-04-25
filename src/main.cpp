// --------  Библиотеки   --------
#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_PCF8574.h>
#include "SparkFun_SCD4x_Arduino_Library.h"

// -------   Пины   -------
#define BTN_PIN   25   // Кнопка подтверждения
#define ENC_F_PIN 32   // Канал A энкодера
#define ENC_L_PIN 33   // Канал B энкодера

// -------   Константы для графиков   -------
#define DEQUE_SIZE_1M 12
#define DEQUE_SIZE_10M 10
#define DEQUE_SIZE_2H 12
#define DEQUE_SIZE_1D 12
#define DEQUE_SIZE_12D 12

// -------   Объект RTC и LCD   -------
RTC_DS3231 rtc;
LiquidCrystal_PCF8574 lcd(0x27);
SCD4x SCD40;

// -------   Шрифты   -------
const byte barChars[8][8] = {
  {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111},
  {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111},
  {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111},
  {0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111},
  {0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111},
  {0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111},
  {0b00000, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111},
  {0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111}
};
const byte ClockChars[8][8] = {
  {0b11111,0b11111,0b11111,0b11111,0b11111,0b11111,0b11111,0b11111},
  {0b11111,0b11111,0b11111,0b11111,0b00000,0b00000,0b00000,0b00000},
  {0b00001,0b00111,0b01111,0b11111,0b11111,0b11111,0b11111,0b11111},
  {0b10000,0b11100,0b11110,0b11111,0b11111,0b11111,0b11111,0b11111},
  {0b00001,0b00111,0b01111,0b11111,0b00000,0b00000,0b00000,0b00000},
  {0b10000,0b11100,0b11110,0b11111,0b00000,0b00000,0b00000,0b00000},
  {0b11111,0b01111,0b00111,0b00001,0b00000,0b00000,0b00000,0b00000},
  {0b11111,0b11110,0b11100,0b10000,0b00000,0b00000,0b00000,0b00000}
};

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
uint8_t menuMode = 0;
uint8_t undermenuMode = 0;
// -------   Переменные, Класс и Деки для работы с метеоданными   ---------
class FloatDeque {
  private:
      float *data;
      int front, rear, size, capacity;
  
  public:
      FloatDeque(const FloatDeque&) = delete;
      FloatDeque& operator=(const FloatDeque&) = delete;
      FloatDeque(int maxSize) {
          capacity = maxSize;
          data = new float[capacity];
          front = 0;
          rear = -1;
          size = 0;
      }
  
      ~FloatDeque() {
          delete[] data;
      }
  
      bool isFull() { return size == capacity; }
      bool isEmpty() { return size == 0; }
  
      void push_front(float value) {
          if (isFull()) pop_back();
          front = (front - 1 + capacity) % capacity;
          data[front] = value;
          size++;
      }
  
      void push_back(float value) {
          if (isFull()) pop_front();
          rear = (rear + 1) % capacity;
          data[rear] = value;
          size++;
      }
  
      void pop_front() {
          if (isEmpty()) return;
          front = (front + 1) % capacity;
          size--;
      }
  
      void pop_back() {
          if (isEmpty()) return;
          rear = (rear - 1 + capacity) % capacity;
          size--;
      }
  
      float average() {
          if (isEmpty()) return 0.0;
          float sum = 0.0;
          for (int i = 0; i < size; i++) {
              int index = (front + i) % capacity;
              sum += data[index];
          }
          return sum / size;
      }
  
      void copyToArray(float* destArray, uint8_t length) {
          for(int i = 0; i<length; i++)
          {
              destArray[i] = 0;
          }
          int copySize = (length < size) ? length : size; // Не копіюємо більше, ніж є в деку
          for (int i = 0; i < copySize; i++) {
              int index = (front + i) % capacity; // Отримуємо правильний індекс у деку
              destArray[copySize - 1 - i] = data[index]; // Копіюємо в зворотному порядку
          }
      }
};
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

// ******************   ФУНКЦИИ   ******************
// -------   Функция обработки энкодера   -------
void updateEncoder() {
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

//  --------   Функции отрисовки часов  --------
void printNumber(uint8_t number,uint8_t col) 
{
   switch (number)
   {
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
void printClock()
{
    if(Clock_Settings_Done == false)
    {
        for (uint8_t i = 0; i < 8; i++) 
        {
            lcd.createChar(i, (uint8_t*)ClockChars[i]);
        }
        Graph_Settings_Done = false;
        Clock_Settings_Done = true;
    }
    if(millis() - lastTimeUpdate >= 1000) 
    {
        lastTimeUpdate = millis();
        DateTime now = rtc.now();
        if(now.hour() < 10)
        {
            printNumber(0,0);
            printNumber(now.hour(),4);
        }
        else{
            printNumber((now.hour()-(now.hour() % 10))/10,0);
            printNumber(now.hour() % 10,4);
        }
        lcd.setCursor(7,1);
        lcd.print(":");
        if(now.minute() < 10)
        {
            printNumber(0,8);
            printNumber(now.minute(),12);
        }
        else{
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

//  --------   Функции изменения времени   -------
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
  long baseEncoder = ENC_Pos;

  while (digitalRead(BTN_PIN) == LOW) {
    delay(10);
  }

  lcd.clear();
  while (!Time_Setting_Done) {
    int step = (ENC_Pos - baseEncoder) / 4;

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
        baseEncoder = ENC_Pos;
      }
    }
    delay(10);
  }
}

// -------   Функции метео данных   -------
void printMeteoData() {
  lcd.setCursor(0,0);
  lcd.print("Meteo data:");
  lcd.setCursor(0,1);
  lcd.print("temp:");
  lcd.setCursor(11,1);
  lcd.print(SCD40.getTemperature() - 1);
  lcd.setCursor(18,1);
  lcd.print("C");
  lcd.setCursor(0,2);
  lcd.print("hum:");
  lcd.setCursor(11,2);
  lcd.print(SCD40.getHumidity());
  lcd.setCursor(17,2);
  lcd.print("%RH");
  lcd.setCursor(0,3);
  lcd.print("CO2:");
  lcd.setCursor(11,3);
  lcd.print(SCD40.getCO2());
  lcd.setCursor(17,3);
  lcd.print("ppm");
}

void writeMeteoData(void *parameter){
  while (true)
  {
    temp_1m.push_front(SCD40.getTemperature() - 1);
    hum_1m.push_front(SCD40.getHumidity());
    CO2_1m.push_front(SCD40.getCO2());
    index_1m++;
    if(index_1m == 10){
      index_1m = 0;
      temp_10m.push_front(temp_1m.average());
      hum_10m.push_front(hum_1m.average());
      CO2_10m.push_front(CO2_1m.average());
      index_10m++;
      if(index_10m == 12){
        index_10m = 0;
        temp_2h.push_front(temp_10m.average());
        hum_2h.push_front(hum_10m.average());
        CO2_2h.push_front(CO2_10m.average());
        index_2h++;
        if(index_2h == 12){
          index_2h = 0;
          temp_1d.push_front(temp_2h.average());
          hum_1d.push_front(hum_2h.average());
          CO2_1d.push_front(CO2_2h.average());
          index_1d++;
          if(index_1d == 12){
            index_1d = 0;
            temp_12d.push_front(temp_1d.average());
            hum_12d.push_front(hum_1d.average());
            CO2_12d.push_front(CO2_1d.average());
          }
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

void drawBarGraph(FloatDeque &data, uint8_t dataSize,uint8_t index, String name, String time) {
    
  if(Graph_Settings_Done == false){ // проверака на наличее шрифтов
      for (uint8_t i = 0; i < 8; i++) {
          lcd.createChar(i, (uint8_t*)barChars[i]);
      }
      Clock_Settings_Done = false;
      Graph_Settings_Done = true;
  }

  data.copyToArray(temp,dataSize);

  if(Graph_Reset == false){
    index_temp = index;
    Graph_Reset = true;
  }

  if(index != index_temp)
  {
    lcd.clear();
    index_temp = index;
  }
  // Знаходимо мінімальне і максимальне значення
  float minVal = temp[0], maxVal = temp[0];
  for (uint8_t i = 1; i < dataSize; i++) {
      if(temp[i]!=0){
          if (temp[i] < minVal) minVal = temp[i];
          if (temp[i] > maxVal) maxVal = temp[i];
      }
  }
  
  lcd.setCursor(16,0);
  lcd.print(name);
  lcd.setCursor(12,1);
  lcd.print(time);
  maxVal=ceil(maxVal);
  minVal=floor(minVal);
  lcd.setCursor(12,2);
  lcd.print("Max:");
  lcd.print((int)maxVal);
  lcd.setCursor(12,3);
  lcd.print("Min:");
  lcd.print((int)minVal);

  if (maxVal == 0) {
      lcd.setCursor(0, 0);
      lcd.print("NO DATA...");
      return;
  }
  
  uint8_t scaled[dataSize];
  for (uint8_t i = 0; i < dataSize; i++) {
      if(temp[i] != 0)
      {
          scaled[i] = ((temp[i]-minVal)*(32)/(maxVal-minVal));
      }
      else{
          scaled[i]=0;
      }
  }
  
  for (uint8_t col = 0; col < dataSize; col++) {
       
      if (scaled[col] == 0) continue;
      
      for (uint8_t row = 0; row < 4; row++) {
          lcd.setCursor(col, 3 - row);
              if (scaled[col] >= 8) {
                  lcd.write(7);
                  scaled[col] -= 8;
              } else {
                  lcd.write(scaled[col] - 1);
                  break;
              }
          
      }
  }
}

// -------   Функции меню   -------
void menu_swich(){
  // Переключение между пунктами меню
  if(digitalRead(BTN_PIN) == LOW){
    delay(200);
    menuMode++;
    undermenuMode = 1;
    if(menuMode > 8) menuMode = 0;
    Graph_Reset = false;
    lcd.clear();
  }
}
void undermenu_swich(uint8_t &undermenuMode, uint8_t high_value, uint8_t low_value){
  // Переключение между подменю
  int step = (ENC_Pos - baseEncoder) / 4;
  if (step != 0) {
    undermenuMode += step;
    baseEncoder += step * 4;
    if (undermenuMode > high_value) undermenuMode = low_value;
    if (undermenuMode < low_value) undermenuMode = high_value;
    Graph_Reset = false;
    lcd.clear();
  }
}
void settings(){
  Settings_Done = false;
  undermenuMode = 6;  // начинаем с первого пункта
  
  while (Settings_Done != true)  // цикл работает, пока не нажата кнопка
  {
    undermenu_swich(undermenuMode, 6, 1);
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
    
    switch (undermenuMode)
    {
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
          lcd.setBacklight(false);// ХЗ
          Sleep = true;
          while (Sleep == true)
          {
            delay(500);
            if(digitalRead(BTN_PIN) == LOW){
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
  
  menuMode = 0;
}
void menu(){
  menu_swich();
  switch (menuMode)
  {
  case 0: // часы
    printClock();
    delay(100);
    break;
  case 1: // метеоданные
    if((millis()/1000)%2==0){
      printMeteoData();
    }
    break;
  case 2: // Температура
    undermenu_swich(undermenuMode, 5, 1);
    switch (undermenuMode)
    {
    case 1:
      drawBarGraph(temp_1m,12,index_1m,"Temp", "1min");
      break;
    case 2:
      drawBarGraph(temp_10m,10,index_10m,"Temp", "10min");
      break;
    case 3:
      drawBarGraph(temp_2h,12,index_2h,"Temp", "2hours");
      break;
    case 4: 
      drawBarGraph(temp_1d,12,index_1d,"Temp", "1day");
      break;
    case 5:
      drawBarGraph(temp_12d,12,index_12d,"Temp", "12days");
      break;
    default:
      break;
    }
  break;
  case 3: // Влажность
    undermenu_swich(undermenuMode, 5, 1);
    switch (undermenuMode)
    {
      case 1:
      drawBarGraph(hum_1m,12,index_1m,"Hum", "1min");
      break;
    case 2:
      drawBarGraph(hum_10m,10,index_10m,"Hum", "10min");
      break;
    case 3:
      drawBarGraph(hum_2h,12,index_2h,"Hum", "2hours");
      break;
    case 4:
      drawBarGraph(hum_1d,12,index_1d,"Hum", "1day");
      break;
    case 5:
      drawBarGraph(hum_12d,12,index_12d,"Hum", "12days");
      break;
    default:
      break;
    }
    break;
  case 4: // CO2
    undermenu_swich(undermenuMode, 5, 1);
    switch (undermenuMode)
    {
      case 1:
      drawBarGraph(CO2_1m,12,index_1m,"CO2", "1min");
      break;
    case 2:
      drawBarGraph(CO2_10m,10,index_10m,"CO2", "10min");
      break;
    case 3:
      drawBarGraph(CO2_2h,12,index_2h,"CO2", "2hours");
      break;
    case 4:
      drawBarGraph(CO2_1d,12,index_1d,"CO2", "1day");
      break;
    case 5:
      drawBarGraph(CO2_12d,12,index_12d,"CO2", "12days");
      break;
    default:
      break;
    }
    break;
  case 5:
    lcd.setCursor(0, 0); 
    lcd.print("MODE 5");
    break;
  case 6:
    lcd.setCursor(0, 0);
    lcd.print("MODE 6");
    break;
  case 7:
    lcd.setCursor(0, 0);
    lcd.print("MODE 7");
    break;
  case 8:
    settings();
    break;
  default:
    break;
  }
}

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

  attachInterrupt(digitalPinToInterrupt(ENC_F_PIN), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_L_PIN), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), menu_swich, LOW);

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
  menu();
}


