#pragma once

#include "../defs.h"

template<class T>
struct Span {
    Span(const T* array, size_t size) {
        items = array;
        len = size;
    }

    const T* items;
    u64 len;

    T& operator[](u64 index) {
        return items[index];
    }
    const T& operator[](u64 index) const {
        return items[index];
    }
};