#pragma once

#include "defs.h"

struct KauCompiler {
    bool m_had_error = false;
    bool m_had_runtime_error = false;

    void error(int line, std::string_view message);
    void runtime_error(int line, std::string_view message);

    int run(char* program, int size);

    int run_prompt();

    int run_file(const char* file_path);
};