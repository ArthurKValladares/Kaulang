#pragma once

#include "../defs.h"

struct Arena {
    Arena();

    void release();

    void* push(u64 size);
    void* push_zero(u64 size);

    template<class T>
    void* push_array(u64 count) {
        push(sizeof(T) * count);
    }
    template<class T>
    void* push_array_zero(u64 count) {
        push_zero(sizeof(T) * count);
    }

    template<class T>
    void* push_struct() {
        push_array<T>(1);
    }
    template<class T>
    void* push_struct_zero() {
        push_array_zero<T>(1);
    }

    void pop(u64 size);

    u64 get_pos() const;

    void set_pos_back(u64 pos);
    void clear();
};