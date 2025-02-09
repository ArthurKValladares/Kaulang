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
        m_head[m_len++] = std::move(item);
    }
    void pop() {
        m_arena->pop(sizeof(T));
        --m_len;
    }

    u64 size() const {
        return m_len;
    }
    bool empty() const {
        return m_len == 0;
    }

    T& operator[](u64 index) {
        return m_head[index];
    }
    const T& operator[](u64 index) const {
        return m_head[index];
    }

    T& back() {
        return m_head[m_len - 1];
    }
    const T& back() const {
        return m_head[m_len - 1];
    }

    T* curr_ptr() {
        return &m_head[m_len];
    }
    void advance() {
        m_arena->push_struct_no_zero<T>();
        m_len++;
    }

    Arena* m_arena;
    T* m_head;
    u64 m_len;
};