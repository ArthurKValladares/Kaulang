#include "defs.h"

#include "scanner.h"

#include <stdio.h>
#include <stdlib.h> 
#include <iostream>

long get_file_size(FILE* file) {
    const long prev = ftell(file);

    fseek(file, 0, SEEK_END);
    const long sz = ftell(file);

    fseek(file, prev, SEEK_SET); // revert to initial position

    return sz;
}

struct KauCompiler {
    bool m_had_error = false;

    void error(int line, const char* message) {
        report(line, "", message);
    }
    void report(int line, const char* where, const char* message) {
        fprintf(stderr, "[Line %d] Error %s: %s\n", line, where, message);
        m_had_error = true;
    }

    int run(char* program, int size) {
        Scanner scanner = Scanner(program, size);
        scanner.scan_tokens();
        
#ifdef DEBUG
        scanner.print_tokens();
#endif

        return 0;
    }

    int run_prompt() {
        constexpr long max_line_size = 1024;
        char line_char_buffer[max_line_size];

        while (true) {
            fprintf(stdout, "> ");

            std::cin.getline(line_char_buffer, max_line_size);
            const int line_size = strlen(line_char_buffer);
            if (line_size == 0) {
                break;
            }

            run(line_char_buffer, line_size);

            if (m_had_error) {
                // TODO: Do I do anything here?
            }
            m_had_error = false;
        }

        return 0;
    }

    int run_file(const char* file_path) {
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

        run(byte_buffer, end);

        if (m_had_error) {
            return -1;
        }

        return 0;
    }
};

int main(int argc, char **argv) {
    KauCompiler kau;
    switch (argc) {
        case 1: {
            kau.run_prompt();
            break;
        }
        case 2: {
            kau.run_file(argv[1]);
            break;
        }
        default: {
            fprintf(stderr, "Usage: kau <path-to-script>");
            return -1;
        }
    }
    return 0;
}