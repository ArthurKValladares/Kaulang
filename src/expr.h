#pragma once

#include "tokens.h"

struct Expr;
struct LiteralExpr;
struct GroupingExpr;
struct UnaryExpr;
struct BinaryExpr;
struct OperatorExpr;

union RulePayload {
    Expr* expression;
    LiteralExpr* literal;
    GroupingExpr* grouping;
    UnaryExpr* unary;
    BinaryExpr* binary;
    OperatorExpr* op;
};

struct Expr {
    enum class Type {
        LITERAL,
        UNARY,
        BINARY,
        GROUPING,
    };

    Type ty;
    RulePayload* expr;
};

struct LiteralExpr {
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

struct GroupingExpr {
    enum class Type {
        PARENS
    };

    Type ty;
    Expr* expr;
};

struct UnaryExpr {
    enum class Type {
        UNARY_MINUS,
        UNARY_BANG,
    };

    Type ty;
    Expr* expr;
};

struct BinaryExpr {
    Expr* left;
    OperatorExpr* op;
    Expr* right;
};

struct OperatorExpr {
    enum class Type {
        EQUAL_EQUAL, BANG_EQUAL,
        LESSER, LESSER_EQUAL,
        GREATER, GREATER_EQUAL,
        PLUS, MINUS, STAR, SLASH
    };

    Type ty;
};