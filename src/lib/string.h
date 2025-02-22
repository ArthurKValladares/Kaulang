#pragma once

#include "arena.h"

#define HASH_STR(str) StringHasher()(str)
#define CREATE_STRING(static_str) String{static_str, ArrayCount(static_str) - 1}

int str_cmp(const char* s1, size_t s1_len, const char* s2, size_t s2_len);

struct String {
    const char* chars = nullptr;
    size_t len = 0;

    bool operator==(const String& other) const
    {
        if (len != other.len) return false;
        return str_cmp(chars, len, other.chars, other.len) == 0;
    }

    bool operator<(const String & other) const {
        return str_cmp(chars, len, other.chars, other.len) < 0;
    }

    bool operator<=(const String & other) const {
        return str_cmp(chars, len, other.chars, other.len) <= 0;
    }

    bool operator>(const String & other) const {
        return str_cmp(chars, len, other.chars, other.len) > 0;
    }

    bool operator>=(const String & other) const {
        return str_cmp(chars, len, other.chars, other.len) >= 0;
    }

    bool empty() const {
        return chars == nullptr;
    }
};

String concatenated_string(Arena* arena, String left, String right);

struct StringHasher {
    size_t operator()(const String& p) const
    {
        size_t hash = 5381;

        for (size_t i = 0; i < p.len; ++i) {
            char c = p.chars[i];
            hash = ((hash << 5) + hash) + c;
        }

        return hash;
    }
};