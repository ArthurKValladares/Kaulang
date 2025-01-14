#include "string.h"

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
    const unsigned char c2 = i1 < s1_len ? s1[i1] : '\0';
    return c1 - c2;
}
