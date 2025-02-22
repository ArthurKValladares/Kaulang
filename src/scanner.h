#pragma once

#include "lib/string.h"
#include "lib/arena.h"
#include "lib/array.h"

#include "tokens.h"
#include "compiler.h"

void init_keywords_map(Arena* arena);

struct Scanner {
    Scanner(Arena* arena, char* source, int len) 
        : m_source(source)
        , m_source_len(len)
    {
        m_tokens.init(arena);
    }

    void scan_tokens(KauCompiler& compiler, Arena* arena);

#ifdef DEBUG
    void print_tokens();
#endif

    Array<Token> m_tokens;
    
private:
    void scan_token(KauCompiler& compiler, Arena* arena);

    bool is_at_end() const;

    char peek() const;
    char peek_next() const;

    char advance();
    bool match(char c);

    String get_substring(int start_offset, int end_offset) const;

    void string(KauCompiler& compiler, Arena* arena);
    void number(KauCompiler& compiler, Arena* arena);
    void identifier(Arena* arena);
    void block_comment(KauCompiler& compiler);
    
    void add_token(Arena* arena, TokenType token_type, String substr = {}, TokenData data = {});

    char* m_source;
    int m_source_len;

    int m_start_char_offset = 0;
    int m_current_char_offset = 0;

    int m_current_line = 1;

    friend class Parser;
};