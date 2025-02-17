#include "arena.h"

#include <windows.h>

namespace {
    u64 round_up_to_multiple(u64 multiple, u64 size) {
        if (size == 0) return 0;

        if (size <= multiple) {
            return multiple;
        } else {
            const u64 remainder = size % multiple;
            return size + (multiple - remainder);
        }
    }
};

Arena* alloc_arena() {
    Arena* arena = (Arena*) malloc(sizeof(Arena));
    assert(arena);

    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);

    arena->page_size = sys_info.dwPageSize;
    arena->mem = VirtualAlloc(nullptr, round_up_to_multiple(arena->page_size, 64 * 1e9), MEM_RESERVE, PAGE_READWRITE);
    const u64 initial_commit_size = arena->page_size * 5;
    void* ret = VirtualAlloc(arena->mem, initial_commit_size, MEM_COMMIT, PAGE_READWRITE);
    assert(ret != nullptr);
    assert(ret == arena->mem);
    arena->commited_size = initial_commit_size;
    arena->offset = 0;

    return arena;
}

void Arena::release() {
    page_size = 0;
    VirtualFree(mem, 0, MEM_RELEASE);
    offset = 0;
}

void* Arena::push(u64 size) {
    void* ret = push_no_zero(size);
    memset(ret, 0, size);
    return ret;
}

void* Arena::push_no_zero(u64 size) {
    if (offset + size > commited_size) {
        u64 alloc_size = round_up_to_multiple(page_size, size);
        void* ret = VirtualAlloc((u8*)mem + (offset + size), alloc_size, MEM_COMMIT, PAGE_READWRITE);
        assert(ret);
        commited_size += alloc_size;
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
    /*
    if (child_arena != nullptr) {
        child_arena->clear();
        free(child_arena);
    }
    */
    offset = 0;
}