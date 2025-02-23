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

void Map::insert(Arena* arena, void* key, u64 key_size, u64 hashed_key, void* object) {
    const u64 bucket = hashed_key % num_buckets;
    MapNode** tmp;
    MapNode*  node;

    tmp = &buckets[bucket];
    while(*tmp != nullptr) {
        if(hashed_key == (*tmp)->hashed_key) {
            break;
        }
        tmp = &(*tmp)->next;
    }

    if(*tmp != nullptr) {
        node = *tmp;
    } else {
        node = (MapNode*) arena->push_struct<MapNode>();
        node->next = nullptr;
        *tmp = node;
    }
    node->key = arena->push_no_zero(key_size);
    memcpy(node->key, key, key_size);

    node->hashed_key = hashed_key;
    node->value = object;
}