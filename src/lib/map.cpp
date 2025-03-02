#include "map.h"

#include <string.h>

#define GET_IMPL() do {\
    const u64 bucket = hashed_key % num_buckets;\
    MapNode* node;\
    node = buckets[bucket];\
    while(node != nullptr) {\
        if(hashed_key == node->hashed_key) {\
            return node->value;\
        }\
        node = node->next;\
    }\
    return nullptr;\
} while(0)

void Map::allocate(Arena* arena) {
    allocate(arena, 256);
}

void Map::allocate(Arena* arena, u64 num_buckets) {
    buckets = (MapNode**) arena->push_array<MapNode*>(num_buckets);
    this->num_buckets = num_buckets;
}

void* Map::get(u64 hashed_key) {
    GET_IMPL();
}

const void* Map::get_const(u64 hashed_key)  const {
    GET_IMPL();
}
