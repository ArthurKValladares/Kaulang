#pragma once

#include "tokens.h"

#include <string>

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

struct Value {
    enum class Type {
        NIL,
        BOOL,
        // TODO: Support other number types
        FLOAT,
        STRING,
    };

    Type ty;
    union {
        bool b;
        float f;
        std::string_view str;
    };
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
    Value evaluate();
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