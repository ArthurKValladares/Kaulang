#pragma once

#include "tokens.h"

struct ExpressionPayload;
struct LiteralPayload;
struct GroupingPayload;
struct UnaryPayload;
struct BinaryPayload;
struct OperatorPayload;

union RulePayload {
    ExpressionPayload* expression;
    LiteralPayload* literal;
    GroupingPayload* grouping;
    UnaryPayload* unary;
    BinaryPayload* binary;
    OperatorPayload* op;
};

struct ExpressionPayload {
    enum class Type {
        LITERAL,
        UNARY,
        BINARY,
        GROUPING,
    };

    Type ty;
    RulePayload* expr;
};

struct LiteralPayload {
    enum class Type {
        LITERAL_NUMBER,
        LITERAL_STRING,
        LITERAL_TRUE,
        LITERAL_FALSE,
        LITERAL_NIL,
    };

    Type ty;
    union {
        float number;
        std::string_view string;
    } data;
};

struct GroupingPayload {
    enum class Type {
        PARENS
    };

    Type ty;
    ExpressionPayload* expr;
};

struct UnaryPayload {
    enum class Type {
        UNARY_MINUS,
        UNARY_BANG,
    };

    Type ty;
    ExpressionPayload* expr;
};

struct BinaryPayload {
    ExpressionPayload* left;
    OperatorPayload* op;
    ExpressionPayload* right;
};

struct OperatorPayload {
    enum class Type {
        EQUAL_EQUAL, BANG_EQUAL,
        LESSER, LESSER_EQUAL,
        GREATER, GREATER_EQUAL,
        PLUS, MINUS, STAR, SLASH
    };

    Type ty;
};