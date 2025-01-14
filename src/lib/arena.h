#pragma once

#include "../defs.h"

struct Arena {
    void alloc(u64 cap);

    void release();

    void* push(u64 size);
    void* push_no_zero(u64 size);

    template<class T>
    void* push_array(u64 count) {
        return push(sizeof(T) * count);
    }
    template<class T>
    void* push_array_no_zero(u64 count) {
        return push_no_zero(sizeof(T) * count);
    }

    template<class T>
    void* push_struct() {
        return push_array<T>(1);
    }
    template<class T>
    void* push_struct_no_zero() {
        return push_array_no_zero<T>(1);
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