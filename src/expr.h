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

struct RuntimeError {
    static RuntimeError ok();
    static RuntimeError unsupported_literal(Token* token);
    static RuntimeError unsupported_binary_op(Token* token);
    static RuntimeError unsupported_unary_op(Token* token);
    static RuntimeError operands_must_be_floats(Token* token);
    static RuntimeError operand_must_be_float(Token* token);
    static RuntimeError operand_must_be_bool(Token* token);

    bool is_ok() const;

    enum class Type {
        Ok,
        UNSUPPORTED_LITERAL,
        UNSUPPORTED_OPERATOR,
        WRONG_OPERANDS,
    };

    Type ty;
    Token* token;
    std::string_view message;
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
    RuntimeError evaluate(Value& in_value);
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