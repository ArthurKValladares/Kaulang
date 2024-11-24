#pragma once

#include <string_view>

enum class TokenType {
    // Single-Character tokens
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

    // One or two character tokens
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESSER, LESSER_EQUAL,

    // Literals
    IDENTIFIER, STRING, NUMBER,

    // Keywords
    AND, CLASS, ELSE, FALSE, FN, FOR, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,

    _EOF
};

struct TokenData {
    enum class Type {
        NIL,
        FLOAT,
        STRING
    };

    Type ty;
    union InnerData {
        float f;
    } data;

    static TokenData new_float(float val);
};

struct Token {
    TokenType m_type;
    std::string_view m_lexeme;
    int m_line;
    TokenData data;

    void print() const;
};

const char* token_type_to_string(TokenType ty);