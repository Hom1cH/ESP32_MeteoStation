#include "FloatDeque.h"

FloatDeque::FloatDeque(int maxSize) {
    if (maxSize <= 0) {
        capacity = 0;
        data = nullptr;
    } else {
        capacity = maxSize;
        data = new float[capacity];
        if (data) {
            // Инициализируем массив нулями
            for (int i = 0; i < capacity; i++) {
                data[i] = 0.0f;
            }
        } else {
            capacity = 0; // Если выделение памяти не удалось
        }
    }
    front = 0;
    rear = -1;
    size = 0;
}

FloatDeque::~FloatDeque() {
    if (data) {
        delete[] data;
        data = nullptr;
    }
}

bool FloatDeque::isFull() { 
    return size == capacity; 
}

bool FloatDeque::isEmpty() { 
    return size == 0; 
}

bool FloatDeque::isValid() { 
    return data != nullptr && capacity > 0; 
}

void FloatDeque::push_front(float value) {
    if (!isValid()) return;
    
    if (isFull()) pop_back();
    front = (front - 1 + capacity) % capacity;
    data[front] = value;
    size++;
}

void FloatDeque::push_back(float value) {
    if (!isValid()) return;
    
    if (isFull()) pop_front();
    rear = (rear + 1) % capacity;
    data[rear] = value;
    size++;
}

void FloatDeque::pop_front() {
    if (isEmpty() || !isValid()) return;
    front = (front + 1) % capacity;
    size--;
}

void FloatDeque::pop_back() {
    if (isEmpty() || !isValid()) return;
    rear = (rear - 1 + capacity) % capacity;
    size--;
}

float FloatDeque::average() {
    if (isEmpty() || !isValid()) return 0.0f;
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        int index = (front + i) % capacity;
        sum += data[index];
    }
    return sum / size;
}

void FloatDeque::copyToArray(float* destArray, uint8_t length) {
    if (!destArray || !isValid()) return;
    
    // Инициализируем массив назначения нулями
    for(int i = 0; i < length; i++) {
        destArray[i] = 0.0f;
    }
    
    int copySize = (length < size) ? length : size;
    for (int i = 0; i < copySize; i++) {
        int index = (front + i) % capacity;
        destArray[copySize - 1 - i] = data[index];
    }
} 