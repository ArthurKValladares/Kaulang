#pragma once

#include "../defs.h"
#include "arena.h"

template<class T>
struct Array {
    void init(Arena* arena, u64 len_to_reserve = 0) {
        m_arena = arena;
        m_head = (T*) arena->push_array_no_zero<T>(len_to_reserve);
        m_len = len_to_reserve;
    }

    void push(T item) {
        m_arena->push_struct_no_zero<T>();
        m_head[m_len++] = item;
    }

    T& operator[](u64 index) {
        return m_head[index];
    }

    const T& operator[](u64 index) const {
        return m_head[index];
    }

    u64 size() const {
        return m_len;
    }

    Arena* m_arena;
    T* m_head;
    u64 m_len;
};