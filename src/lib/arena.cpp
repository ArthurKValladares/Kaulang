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

    // NOTE: Having a zero sized node at the start makes things easier
    FreeNode* zero_node = (FreeNode*) malloc(sizeof(FreeNode));
    zero_node->size = 0;
    zero_node->prev = nullptr;
    zero_node->next = nullptr;

    arena->free_list_head = zero_node;
    arena->free_list_tail = zero_node;

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

void Arena::free_section(void* start, u64 size) {
    FreeNode* free_node = (FreeNode*) malloc(sizeof(FreeNode));
    *free_node = FreeNode {
        .head = start,
        .size = size,
        .prev = nullptr,
        .next = nullptr,
    };
    
    free_list_tail->next = free_node;

    free_list_tail = free_node;
}

void* Arena::get_from_list_with_size(u64 size) {
    if (size == 0) return nullptr;

    FreeNode* test_node = free_list_head;
    while(test_node->size < size) {
        if (test_node->next == nullptr) {
            return nullptr;
        }
        test_node = test_node->next;
    }

    void* ret;
    const u64 remaining_size = test_node->size - size;
    if (remaining_size == 0) {
        test_node->prev->next = test_node->next;
        ret = test_node->head;

        free(test_node);
    } else {
        test_node->size -= size;
        ret = (u8*) test_node->head + test_node->size;
    }

    return ret;
}