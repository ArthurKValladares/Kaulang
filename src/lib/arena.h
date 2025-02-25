#pragma once

#include "../defs.h"

struct FreeNode {
    void* head;
    u64 size;
    FreeNode* prev;
    FreeNode* next;
};

// TODO: in general for this arena and for the array that uses it,
// I think I'm not worrying enough about memory-aligment
struct Arena {
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

    void free_section(void* start, u64 size);

    void* get_from_list_with_size(u64 size);

    // TODO: This is pretty sloppy and i need to handle multiple arenas better later,
    // This is here just to make vectors work
    Arena* child_arena;

    u64 page_size;
    u64 commited_size;

    void* mem;
    u64 offset;

    FreeNode* free_list_head = nullptr;
    FreeNode* free_list_tail = nullptr;
};

Arena* alloc_arena();