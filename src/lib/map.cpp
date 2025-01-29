#include "map.h"

void StringMap::allocate(Arena* arena) {
    buckets = (MapNode**) arena->push_array_no_zero<MapNode*>(STRING_MAP_NUM_BUCKETS);
}

void* StringMap::get(String key) {
    u64 bucket = StringHasher()(key) % STRING_MAP_NUM_BUCKETS;
    MapNode* node;
    node = buckets[bucket];
    while(node) {
        if(key == node->key) {
            return node->value;
        }
        node = node->next;
    }
    return nullptr;
}

void StringMap::insert(Arena* arena, String key, void* object) {
    u64 bucket = StringHasher()(key) % STRING_MAP_NUM_BUCKETS;
    MapNode** tmp;
    MapNode*  node;

    tmp = &buckets[bucket];
    while(*tmp) {
        if(key == (*tmp)->key) {
            break;
        }
        tmp = &(*tmp)->next;
    }

    if(*tmp) {
        node = *tmp;
    } else {
        node = (MapNode*) arena->push_struct<MapNode>();
        node->next = nullptr;
        *tmp = node;
    }
    node->key = key;
    node->value = object;
}