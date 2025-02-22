#include "string.h"

#include <string.h>

int str_cmp(const char* s1, size_t s1_len, const char* s2, size_t s2_len)
{
    size_t i1 = 0;
    size_t i2 = 0;
    while(i1 < s1_len && i2 < s2_len && (s1[i1] == s2[i2]))
    {
        ++i1;
        ++i2;
    }
    const unsigned char c1 = i1 < s1_len ? s1[i1] : '\0';
    const unsigned char c2 = i2 < s2_len ? s2[i2] : '\0';
    return c1 - c2;
}

String concatenated_string(Arena* arena, String left, String right) {
    size_t total_len = left.len + right.len;
    char* string_chars = (char*) arena->push_array_no_zero<char>(total_len);
    memcpy(string_chars, left.chars, left.len * sizeof(char));
    memcpy(string_chars + left.len, right.chars, right.len * sizeof(char));

    String ret;
    ret.len = total_len;
    ret.chars = string_chars;

    return String {
        .chars = string_chars,
        .len = total_len
    };
}

String concatenated_strings(Arena* arena, Span<const String*> strings) {
    size_t total_len = 0;
    for (u64 i = 0; i < strings.len; ++i) {
        total_len += strings.len;
    }

    char* string_chars = (char*) arena->push_array_no_zero<char>(total_len);
    u64 offset = 0;
    for (u64 i = 0; i < strings.len; ++i) {
        memcpy(string_chars + offset, strings.items[i]->chars, strings.items[i]->len * sizeof(char));
    }

    String ret;
    ret.len = total_len;
    ret.chars = string_chars;

    return String {
        .chars = string_chars,
        .len = total_len
    };
}