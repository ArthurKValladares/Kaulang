#include "compiler.h"

#include "scanner.h"
#include "parser.h"
#include "resolver.h"

#include <iostream>
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
    global_arena = alloc_arena();

    global_env.init(global_arena);
    
    String clock_str = CREATE_STRING("clock");
    global_env.define_callable(global_arena, clock_str, Callable(0, [](Array<Value> args, KauCompiler* compiler, Arena*, Environment* env) {
        return Value{
            .ty = Value::Type::LONG,
            .l = clock()
        };
    }));

    String print_str = CREATE_STRING("print");
    global_env.define_callable(global_arena, print_str, Callable(1, [](Array<Value> args, KauCompiler* compiler, Arena*, Environment* env) {
        const Value& val = args[0];
        val.print();
        return val;
    }));

    locals.allocate(global_arena);
}

void KauCompiler::error(int line, String message) {
    fprintf(stderr, "[Line %d] Error: %.*s\n", line, (u32) message.len, message.chars);
    m_had_error = true;
}

void KauCompiler::runtime_error(int line, String message) {
    fprintf(stderr, "[Line %d] Runtime Error: %.*s\n", line, (u32) message.len, message.chars);
    m_had_runtime_error = true;
}

int KauCompiler::run(char* program, int size, bool from_prompt) {
    Scanner scanner = Scanner(global_arena, program, size);
    scanner.scan_tokens(*this, global_arena);

    Parser parser(scanner.m_tokens);
    Array<Stmt> stmts = parser.parse(global_arena);
    
    Arena* resolver_arena = alloc_arena();
    Resolver resolver = {};
    resolver.init(resolver_arena);
    resolver.resolve(this, stmts);
    if (m_had_error) {
        return -1;
    }
    resolver_arena->clear();
    
    for (u64 i = 0; i < stmts.size(); ++i) {
        Stmt& stmt = stmts[i];

#ifdef DEBUG_PRINT
        stmt.print();
#endif
        Value val = stmt.evaluate(this, this->global_arena, &global_env, from_prompt, false);
    }

    return 0;
}

int KauCompiler::run_prompt() {
    Arena* program_arena = alloc_arena();

    size_t line_start_offset = 0;
    Array<char> line_char_buffer;
    line_char_buffer.init(program_arena);

    while (true) {
        fprintf(stdout, "> ");

        int line_size = 0;
        int c;
        while (true) {
            c = fgetc(stdin);
            if (c == EOF || c == '\n') {
                break;
            }
            line_char_buffer.push(c);
            ++line_size;
        }

        
        if (line_size == 0) {
            break;
        }

        run(&line_char_buffer[line_start_offset], line_size, true);
        line_start_offset += line_size;

        m_had_error = false;
    }

    global_arena->clear();
    program_arena->clear();

    return 0;
}

int KauCompiler::run_file(const char* file_path) {
    FILE* script_file;
    script_file = fopen(file_path, "r");
    if (script_file == NULL) {
        fprintf(stderr, "Failed to open kau script at: %s\n", file_path);
        return -1;
    }
    const long file_size_bytes = get_file_size(script_file);

    // NOTE: Making it null-terminated for convenience, probably don't need to do it.
    char* byte_buffer = (char*)malloc(file_size_bytes + 1);
    if (byte_buffer == NULL) {
        fprintf(stderr, "Could not allocate byte buffer to store file data.\n");
        return -1;
    }
    const int end = fread(byte_buffer, 1, file_size_bytes, script_file);
    byte_buffer[end] = '\0';

    fclose(script_file);

    run(byte_buffer, end, false);
    global_arena->clear();
    
    if (m_had_error) {
        return -1;
    }
    if (m_had_runtime_error) {
        return -2;
    }

    return 0;
}

RuntimeError KauCompiler::lookup_variable(Environment* env, const Token* name, Expr* expr, Value& in_value) {
    u64* dist = (u64*) locals.get((u64) expr);
    Value* val;
    if (dist != nullptr) {
        val = env->get_at(name->m_lexeme, *dist);
    } else {
        val = global_env.get(name->m_lexeme);
    }

    if (val == nullptr) {
        return RuntimeError::undefined_variable(name);
    }
    if (val->ty == Value::Type::NIL) {
        return RuntimeError::undefined_variable(name);
    }

    in_value = *val;
    return RuntimeError::ok();
}