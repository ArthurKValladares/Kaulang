#pragma once

#include "defs.h"

#include "environment.h"

struct KauCompiler {
    bool m_had_error = false;
    bool m_had_runtime_error = false;
    Environment global_env = {};
    
    void error(int line, std::string_view message);
    void runtime_error(int line, std::string_view message);

    int run(char* program, int size);

    int run_prompt();

    int run_file(const char* file_path);
};