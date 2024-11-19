#include <stdint.h>

#ifdef DEBUG
#include <print>
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

void debug_log(const char* message) {
#ifdef DEBUG
    std::println("LOG: %s", message);
#endif
}