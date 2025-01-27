// SimpleMap.h
#ifndef SIMPLE_MAP_H
#define SIMPLE_MAP_H

#include <Arduino.h>

template<typename K, typename V, int MaxSize>
class SimpleMap {
private:
    K keys[MaxSize];
    V values[MaxSize];
    int size;

public:
    SimpleMap() : size(0) {}

    bool insert(const K& key, const V& value) {
        if (size >= MaxSize) return false;
        for (int i = 0; i < size; i++) {
            if (keys[i] == key) {
                values[i] = value;
                return true;
            }
        }
        keys[size] = key;
        values[size] = value;
        size++;
        return true;
    }

    V* find(const K& key) {
        for (int i = 0; i < size; i++) {
            if (keys[i] == key) {
                return &values[i];
            }
        }
        return nullptr;
    }

    bool remove(const K& key) {
        for (int i = 0; i < size; i++) {
            if (keys[i] == key) {
                for (int j = i; j < size - 1; j++) {
                    keys[j] = keys[j + 1];
                    values[j] = values[j + 1];
                }
                size--;
                return true;
            }
        }
        return false;
    }

    int getSize() const { return size; }

    K keyAt(int index) const {
        if (index >= 0 && index < size) {
            return keys[index];
        }
        return K(); // Returns a default value if the index is out of range
    }

    V valueAt(int index) const {
        if (index >= 0 && index < size) {
            return values[index];
        }
        return V(); // Returns a default value if the index is out of range
    }

    V* getByIndex(int index) {
        if (index >= 0 && index < size) {
            return &values[index];
        }
        return nullptr;
    }


};

#endif // SIMPLE_MAP_H