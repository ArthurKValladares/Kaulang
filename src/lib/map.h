#pragma once

#include "../defs.h"
#include "arena.h"

struct MapNode  {
    void* key;
    u64 hashed_key;
    void* value;
    MapNode* next;
};

struct Map {
    void allocate(Arena* arena);
    void allocate(Arena* arena, u64 num_buckets);

    void* get(u64 hashed_key);
    const void* get_const(u64 hashed_key) const;
    void insert(Arena* arena, const void* key, u64 key_size, u64 hashed_key, const void* object, u64 object_size);

    MapNode** buckets;
    u64 num_buckets;
};