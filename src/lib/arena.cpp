#include "arena.h"

Arena* alloc_arena(u64 cap) {
    Arena* arena = (Arena*) malloc(sizeof(Arena));

    arena->m_cap = cap;
    arena->mem = malloc(cap);
    arena->offset = 0;

    return arena;
}

void Arena::release() {
    m_cap = 0;
    free(mem);
    offset = 0;
}

void* Arena::push(u64 size) {
    void* ret = push(size);
    std::memset(ret, 0, size);
    return ret;
}

void* Arena::push_no_zero(u64 size) {
    if (offset + size > m_cap) {
        fprintf(stderr, "Area ran out of memory.");
        exit(-1);
    }

    void* start_address = (void*) (((u8*) mem) + offset);
    offset += size;

    return start_address;
}

void Arena::pop(u64 size) {
    offset -= size;
}

void Arena::pop_to(u64 pos) {
    offset = pos;
}

u64 Arena::get_pos() const {
    return offset;
}

void Arena::clear() {
    offset = 0;
}