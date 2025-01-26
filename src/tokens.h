#pragma once

#include "lib/string.h"

enum class TokenType {
    // Single-Character tokens
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, COLON, SLASH, STAR,
    QUESTION_MARK,

    // One or two character tokens
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESSER, LESSER_EQUAL,

    // Literals
    IDENTIFIER, STRING,
    NUMBER_INT, NUMBER_LONG, NUMBER_FLOAT, NUMBER_DOUBLE,

    // Keywords
    AND, CLASS, ELSE, FALSE, FN, FOR, IF, NIL, OR,
    RETURN, SUPER, THIS, TRUE, VAR, WHILE,
    BREAK, CONTINUE,

    _EOF
};

struct TokenData {
    enum class Type {
        NIL,
        FLOAT,
        DOUBLE,
        INT,
        LONG,
        STRING
    };

    Type ty;
    union InnerData {
        float f;
        double d;
        int i;
        long l;
    } data;

    static TokenData new_float(float val);
    static TokenData new_double(double val);
    static TokenData new_int(int val);
    static TokenData new_long(long val);
};

struct Token {
    TokenType m_type;
    String m_lexeme;
    int m_line;
    TokenData data;

    void print() const;
};

const char* token_type_to_string(TokenType ty);