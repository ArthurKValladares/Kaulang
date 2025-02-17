#pragma once

#include "../defs.h"
#include "arena.h"

// TODO: Not a particularly good map implementation, but it works for now
#define NUM_BUCKETS 1024

struct MapNode  {
    u64 hashed_key;
    void* value;
    MapNode* next;
};

struct Map {
    void allocate(Arena* arena);

    void* get(u64 hashed_key);
    void insert(Arena* arena, u64 hashed_key, void* object);

    MapNode** buckets;
};