#pragma once

struct KauCompiler {
    bool m_had_error = false;

    void error(int line, const char* message);
    void report(int line, const char* where, const char* message);

    int run(char* program, int size);

    int run_prompt();

    int run_file(const char* file_path);
};