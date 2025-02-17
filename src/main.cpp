#include "defs.h"

#include "compiler.h"
#include "scanner.h"

int main(int argc, char **argv) {
    KauCompiler kau;
    init_keywords_map(kau.global_arena);
    kau.run_file("../test.kau");
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
            fprintf(stderr, "Usage: kau <path-to-script>\n");
            return -1;
        }
    }
    return 0;
}