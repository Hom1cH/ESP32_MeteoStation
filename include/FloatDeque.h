#ifndef FLOAT_DEQUE_H
#define FLOAT_DEQUE_H

#include <Arduino.h>

class FloatDeque {
  private:
      float *data;
      int front, rear, size, capacity;
  
  public:
      FloatDeque(const FloatDeque&) = delete;
      FloatDeque& operator=(const FloatDeque&) = delete;
      
      FloatDeque(int maxSize);
      ~FloatDeque();
  
      bool isFull();
      bool isEmpty();
      bool isValid();
  
      void push_front(float value);
      void push_back(float value);
      void pop_front();
      void pop_back();
      float average();
      void copyToArray(float* destArray, uint8_t length);
};

#endif // FLOAT_DEQUE_H 