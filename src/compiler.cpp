#include "compiler.h"

#include "scanner.h"
#include "parser.h"
#include "resolver.h"

#include <iostream>
#include <print>
#include <ctime>

namespace {
long get_file_size(FILE* file) {
    const long prev = ftell(file);

    fseek(file, 0, SEEK_END);
    const long sz = ftell(file);

    fseek(file, prev, SEEK_SET); // revert to initial position

    return sz;
}
};

KauCompiler::KauCompiler() {
    String clock_str = String {
        .chars = "CLOCK",
        .len = 5
    };
    global_env.define_callable(clock_str, Callable(0, [](std::vector<Value> const &args, KauCompiler* compiler, Arena*, Environment* env) {
        return Value{
            .ty = Value::Type::LONG,
            .l = clock()
        };
    }));
    global_arena = alloc_arena();
}

void KauCompiler::error(int line, std::string_view message) {
    std::println(stderr, "[Line {}] Error: {}", line, message);
    m_had_error = true;
}

void KauCompiler::runtime_error(int line, std::string_view message) {
    std::println(stderr, "[Line {}] Runtime Error: {}", line, message);
    m_had_runtime_error = true;
}

int KauCompiler::run(char* program, int size, bool from_prompt) {
    Scanner scanner = Scanner(global_arena, program, size);
    // TODO: fix this kinda circular dependency thing later
    scanner.scan_tokens(*this, global_arena);

    Parser parser(scanner.m_tokens, scanner.m_tokens_len);
    u64 stmts_len;
    Stmt* stmts = parser.parse(global_arena, stmts_len);

    Resolver resolver = {};
    resolver.resolve(stmts, stmts_len);

    for (u64 i = 0; i < stmts_len; ++i) {
        Stmt& stmt = stmts[i];

#ifdef DEBUG_PRINT
        stmt.print();
#endif
        Value val = stmt.evaluate(this, this->global_arena, &global_env, from_prompt, false);
    }

    global_arena->clear();

    return 0;
}

int KauCompiler::run_prompt() {
    constexpr long max_line_size = 1024;
    char line_char_buffer[max_line_size];

    while (true) {
        std::print("> ");

        std::cin.getline(line_char_buffer, max_line_size);
        const int line_size = strlen(line_char_buffer);
        if (line_size == 0) {
            break;
        }

        run(line_char_buffer, line_size, true);

        if (m_had_error) {
            // TODO: Do I do anything here?
        }
        m_had_error = false;
    }

    return 0;
}

int KauCompiler::run_file(const char* file_path) {
    FILE* script_file;
    script_file = fopen(file_path, "r");
    if (script_file == NULL) {
        std::println(stderr, "Failed to open kau script at: {}", file_path);
        return -1;
    }
    const long file_size_bytes = get_file_size(script_file);

    // NOTE: Making it null-terminated for convenience, probably don't need to do it.
    char* byte_buffer = (char*)malloc(file_size_bytes + 1);
    if (byte_buffer == NULL) {
        std::println(stderr, "Could not allocate byte buffer to store file data.");
        return -1;
    }
    const int end = fread(byte_buffer, 1, file_size_bytes, script_file);
    byte_buffer[end] = '\0';

    fclose(script_file);

    run(byte_buffer, end, false);

    if (m_had_error) {
        return -1;
    }
    if (m_had_runtime_error) {
        return -2;
    }

    return 0;
}