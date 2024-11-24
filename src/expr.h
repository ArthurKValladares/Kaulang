#pragma once

#include "tokens.h"

struct Expr;
struct LiteralExpr;
struct GroupingExpr;
struct UnaryExpr;
struct BinaryExpr;

union ExprPayload {
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

    void print();
};

struct LiteralExpr {
    Token* val;
};

struct GroupingExpr {
    Expr* expr;
};

struct UnaryExpr {
    Token* op;
    Expr* right;
};

struct BinaryExpr {
    Expr* left;
    Token* op;
    Expr* right;
};