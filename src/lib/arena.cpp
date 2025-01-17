#include "arena.h"

#include <windows.h>

namespace {
    size_t round_up_to_multiple(size_t multiple, size_t size) {
        if (size == 0) return 0;

        if (size <= multiple) {
            return multiple;
        } else {
            const size_t remainder = size % multiple;
            return size + (multiple - remainder);
        }
    }
};

Arena* alloc_arena() {
    Arena* arena = (Arena*) malloc(sizeof(Arena));

    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);

    arena->page_size = sys_info.dwPageSize;
    arena->mem = VirtualAlloc(nullptr, arena->page_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    arena->offset = 0;

    return arena;
}

void Arena::release() {
    page_size = 0;
    free(mem);
    offset = 0;
}

void* Arena::push(u64 size) {
    void* ret = push_no_zero(size);
    std::memset(ret, 0, size);
    return ret;
}

void* Arena::push_no_zero(u64 size) {
    if (offset + size > page_size) {
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
    if (child_arena != nullptr) {
        child_arena->clear();
        free(child_arena);
    }
    offset = 0;
}