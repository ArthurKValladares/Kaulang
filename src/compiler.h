#pragma once

#include "lib/arena.h"
#include "defs.h"

#include "environment.h"

struct KauCompiler {
    KauCompiler();
    
    bool m_had_error = false;
    bool m_had_runtime_error = false;
    Environment global_env = {};
    
    void error(int line, std::string_view message);
    void runtime_error(int line, std::string_view message);

    int run(char* program, int size, bool from_prompt);

    int run_prompt();

    int run_file(const char* file_path);

    // TODO: Re-think this, at least make it a bitfield with other stuff like `should_print`
    bool hit_return = false;
    Arena global_arena;
};