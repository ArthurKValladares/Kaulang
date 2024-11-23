#pragma once

#include "tokens.h"

struct Expr;
struct LiteralExpr;
struct GroupingExpr;
struct UnaryExpr;
struct BinaryExpr;

union ExprPayload {
    Expr* expression;
    LiteralExpr* literal;
    GroupingExpr* grouping;
    UnaryExpr* unary;
    BinaryExpr* binary;
};

struct Expr {
    enum class Type {
        LITERAL,
        UNARY,
        BINARY,
        GROUPING,
    };

    Type ty;
    ExprPayload expr;
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
    Token* op;
    Expr* right;
};