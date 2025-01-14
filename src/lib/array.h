#pragma once

#include "arena.h"

template<class T>
struct DynArray {
    DynArray<T>& operator=(const DynArray<T>& that) {
        if (this != that) {
            count = that.count;
            capacity = that.capacity;

            items = realloc(items, capacity * sizeof(T));

            for (size_t i = 0; i < capacity; ++i) {
                items[i] = that.items[i];
            }
        }

        return *this;
    }

    void append(T item) {
        if (count >= capacity) {
            if (capacity == 0) {
                capacity = INITIAL_CAPACITY;
            } else {
                capacity *= 2;
            }
            items = realloc(items, capacity * sizeof(T));
        }
        items[count++] = item;
    }

    T& operator[](int index) {
        return items[index];
    }
    
    size_t size() const {
        return count;
    }

private:
    T* items = nullptr;
    size_t count = 0;
};