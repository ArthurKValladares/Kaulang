#include "defs.h"

#include "compiler.h"

#include <print>

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
            std::println(stderr, "Usage: kau <path-to-script>");
            return -1;
        }
    }
    return 0;
}

// TODO:Literal tokens a bit hard to work with
// maybe add specific literal type to enum case, i.e,
// TokenType::FALSE, TokenType::TRUE, TokenType::NIL, TokenType::NUMBER, TokenType::STRING
// LITERAL_FALSE, LITERAL_TRUE, ...