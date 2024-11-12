#pragma once

#include "tokens.h"

#include <vector>

struct Scanner {
    Scanner(char* source, int len) 
        : m_source(source)
        , m_source_len(len)
    {}

    void scan_tokens();

#ifdef DEBUG
    void print_tokens();
#endif
private:
    void scan_token();

    bool is_at_end() const;

    char peek() const;
    char peek_next() const;

    char advance();
    bool match(char c);

    std::string_view get_substring(int start_offset, int end_offset) const;

    void string();
    void number();
    void identifier();

    void add_token(TokenType token_type);
    void add_token(TokenType token_type, std::string_view substr);

    char* m_source;
    int m_source_len;

    int m_start_char_offset = 0;
    int m_current_char_offset = 0;

    int m_current_line = 1;

    std::vector<Token> m_tokens;
};