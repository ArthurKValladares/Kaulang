#include "defs.h"

#include "compiler.h"

#include <print>

int main(int argc, char **argv) {
    KauCompiler kau;
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
            std::println(stderr, "Usage: kau <path-to-script>");
            return -1;
        }
    }
    return 0;
}
// TODO: print(clock()); is not working