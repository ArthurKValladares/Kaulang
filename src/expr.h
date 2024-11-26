#pragma once

#include "tokens.h"

struct Expr;
struct LiteralExpr;
struct GroupingExpr;
struct UnaryExpr;
struct BinaryExpr;
struct CommaExpr;
struct TernaryExpr;

union ExprPayload {
    LiteralExpr* literal;
    GroupingExpr* grouping;
    UnaryExpr* unary;
    BinaryExpr* binary;
    CommaExpr* comma;
    TernaryExpr* ternary;
};

struct Expr {
    enum class Type {
        LITERAL,
        UNARY,
        BINARY,
        GROUPING,
        COMMA,
        TERNARY,
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

struct CommaExpr {
    Expr* left;
    Expr* right;
};

struct TernaryExpr {
    Expr* left;
    Expr* middle;
    Expr* right;
};