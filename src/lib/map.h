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
    template<class K, class V>
    void insert(Arena* arena, const K& key, u64 hashed_key, const V& value) {
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
        node->key = arena->push_struct_no_zero<K>();
        *(K*)node->key = key;

        node->hashed_key = hashed_key;
        node->value = arena->push_struct_no_zero<V>();
        *(V*)node->value = value;
    }

    MapNode** buckets;
    u64 num_buckets;
};