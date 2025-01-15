#pragma once

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
    size_t capacity = 0;

    const size_t INITIAL_CAPACITY = 32;
};