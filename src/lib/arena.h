#pragma once

#include "../defs.h"

struct Arena {
    Arena(u64 cap);

    void release();

    void* push(u64 size);
    void* push_no_zero(u64 size);

    template<class T>
    void* push_array(u64 count) {
        push(sizeof(T) * count);
    }
    template<class T>
    void* push_array_no_zero(u64 count) {
        push_no_zero(sizeof(T) * count);
    }

    template<class T>
    void* push_struct() {
        push_array<T>(1);
    }
    template<class T>
    void* push_struct_no_zero() {
        push_array_no_zero<T>(1);
    }

    void pop(u64 size);
    void pop_to(u64 pos);

    u64 get_pos() const;

    void clear();

private:
    u64 m_cap;
    void* mem;
    u64 offset;
};