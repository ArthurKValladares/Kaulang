#pragma once

#include "tokens.h"
#include "compiler.h"

#include <vector>

struct Scanner {
    Scanner(char* source, int len) 
        : m_source(source)
        , m_source_len(len)
    {}

    void scan_tokens(KauCompiler& compiler);

#ifdef DEBUG
    void print_tokens();
#endif
private:
    void scan_token(KauCompiler& compiler);

    bool is_at_end() const;

    char peek() const;
    char peek_next() const;

    char advance();
    bool match(char c);

    std::string_view get_substring(int start_offset, int end_offset) const;

    void string(KauCompiler& compiler);
    void number();
    void identifier();
    void block_comment(KauCompiler& compiler);
    
    void add_token(TokenType token_type, TokenData data = {});
    void add_token(TokenType token_type, std::string_view substr, TokenData data = {});

    char* m_source;
    int m_source_len;

    int m_start_char_offset = 0;
    int m_current_char_offset = 0;

    int m_current_line = 1;

    std::vector<Token> m_tokens;
};