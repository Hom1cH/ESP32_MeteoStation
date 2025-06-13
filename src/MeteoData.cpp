#include "MeteoData.h"
#include "Config.h"
#include "Fonts.h"
#include "SparkFun_SCD4x_Arduino_Library.h"

extern SCD4x SCD40;
extern bool Graph_Settings_Done;
extern bool Clock_Settings_Done;
extern bool Graph_Reset;
extern uint8_t index_temp;

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

void writeMeteoData(void *parameter) {
  while (true) {
    temp_1m.push_front(SCD40.getTemperature() - 1);
    hum_1m.push_front(SCD40.getHumidity());
    CO2_1m.push_front(SCD40.getCO2());
    index_1m++;
    if(index_1m == 10) {
      index_1m = 0;
      temp_10m.push_front(temp_1m.average());
      hum_10m.push_front(hum_1m.average());
      CO2_10m.push_front(CO2_1m.average());
      index_10m++;
      if(index_10m == 12) {
        index_10m = 0;
        temp_2h.push_front(temp_10m.average());
        hum_2h.push_front(hum_10m.average());
        CO2_2h.push_front(CO2_10m.average());
        index_2h++;
        if(index_2h == 12) {
          index_2h = 0;
          temp_1d.push_front(temp_2h.average());
          hum_1d.push_front(hum_2h.average());
          CO2_1d.push_front(CO2_2h.average());
          index_1d++;
          if(index_1d == 12) {
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

void drawBarGraph(FloatDeque &data, uint8_t dataSize, uint8_t index, String name, String time) {
    
  if(Graph_Settings_Done == false) { // проверака на наличее шрифтов
      for (uint8_t i = 0; i < 8; i++) {
          lcd.createChar(i, (uint8_t*)barChars[i]);
      }
      Clock_Settings_Done = false;
      Graph_Settings_Done = true;
  }

  data.copyToArray(temp, dataSize);

  if(Graph_Reset == false) {
    index_temp = index;
    Graph_Reset = true;
  }

  if(index != index_temp) {
    lcd.clear();
    index_temp = index;
  }
  
  // Знаходимо мінімальне і максимальне значення
  float minVal = temp[0], maxVal = temp[0];
  for (uint8_t i = 1; i < dataSize; i++) {
      if(temp[i] != 0) {
          if (temp[i] < minVal) minVal = temp[i];
          if (temp[i] > maxVal) maxVal = temp[i];
      }
  }
  
  lcd.setCursor(16,0);
  lcd.print(name);
  lcd.setCursor(12,1);
  lcd.print(time);
  maxVal = ceil(maxVal);
  minVal = floor(minVal);
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
      if(temp[i] != 0) {
          scaled[i] = ((temp[i] - minVal) * (32) / (maxVal - minVal));
      } else {
          scaled[i] = 0;
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