#pragma once

#include "string.h"

struct StringMap {
    StringMap() {}

    bool contains(String key) {
        return false;
    }

    void* get(String key) {
        return nullptr;
    }

    void insert(String key, void* object) {
    }
};