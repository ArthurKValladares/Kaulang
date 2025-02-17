#include "map.h"

void Map::allocate(Arena* arena) {
    buckets = (MapNode**) arena->push_array<MapNode*>(NUM_BUCKETS);
}

void* Map::get(u64 hashed_key) {
    const u64 bucket = hashed_key % NUM_BUCKETS;
    MapNode* node;
    node = buckets[bucket];
    while(node != nullptr) {
        if(hashed_key == node->hashed_key) {
            return node->value;
        }
        node = node->next;
    }
    return nullptr;
}

const void* Map::get(u64 hashed_key) const {
    return get(hashed_key);
}

void Map::insert(Arena* arena, u64 hashed_key, void* object) {
    const u64 bucket = hashed_key % NUM_BUCKETS;
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
    node->hashed_key = hashed_key;
    node->value = object;
}