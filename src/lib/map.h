#pragma once

#include "string.h"
#include "arena.h"

// TODO: Not a particularly good map implementation, but it works for now
#define STRING_MAP_NUM_BUCKETS 1024

struct MapNode  {
    String key;
    void* value;
    MapNode* next;
};

struct StringMap {
    void allocate(Arena* arena);

    void* get(String key);
    void insert(Arena* arena, String key, void* object);

    MapNode** buckets;
};