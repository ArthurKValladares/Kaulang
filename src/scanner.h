#pragma once

#include "tokens.h"

#include <vector>

struct Scanner {
    Scanner(char* source, int len) 
        : m_source(source)
        , m_source_len(len)
    {}

    void scan_tokens();
    void scan_token();

    bool is_at_end() const;

    char peek() const;
    char advance();
    bool match(char c);

    void add_token(TokenType token_type);

    char* m_source;
    int m_source_len;

    int m_start_char_offset = 0;
    int m_current_char_offset = 0;

    int m_current_line = 1;

    std::vector<Token> m_tokens;
};