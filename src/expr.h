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
struct PrintExpr;

union ExprPayload {
    LiteralExpr* literal;
    GroupingExpr* grouping;
    UnaryExpr* unary;
    BinaryExpr* binary;
    CommaExpr* comma;
    TernaryExpr* ternary;
    PrintExpr* print;
};

struct RuntimeError {
    static RuntimeError ok();
    static RuntimeError unsupported_literal(Token* token);
    static RuntimeError unsupported_binary_op(Token* token);
    static RuntimeError unsupported_unary_op(Token* token);
    static RuntimeError operands_must_be_equal(Token* token);
    static RuntimeError operands_must_be_floats(Token* token);
    static RuntimeError operands_must_be_strings(Token* token);
    static RuntimeError operand_must_be_float(Token* token);
    static RuntimeError operand_must_be_bool(Token* token);
    static RuntimeError divide_by_zero(Token* token);
    static RuntimeError operands_do_not_support_operator(Token* token);
    static RuntimeError undefined_variable(Token* token);

    bool is_ok() const;

    enum class Type {
        Ok,
        UNSUPPORTED_LITERAL,
        UNSUPPORTED_OPERATOR,
        WRONG_OPERANDS,
        DIVIDE_BY_ZERO,
        UNDEFINED_VARIABLE,
    };

    Type ty;
    Token* token;
    std::string_view message;
};

struct Value {
    enum class Type {
        NIL,
        BOOL,
        FLOAT,
        DOUBLE,
        INT,
        STRING,
    };

    Type ty;
    union {
        bool b;
        int i;
        float f;
        double d;
        std::string_view str = {};
    };

    void print() const;
};

// This struct is a bit sloppy with weird distinction between the data
// for each type, make better later
struct Stmt {
    enum class Type {
        ERR,
        PRINT,
        EXPR,
        VAR_DECL,
    };

    Type ty;
    Token* name;
    Expr* expr;

    void print();
};

struct Environment;
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

    void print() const;
    RuntimeError evaluate(Environment& env, Value& in_value);
};

// TODO: review these Token*'s later
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
    Token* op;
    Expr* right;
};

struct TernaryExpr {
    Expr* left;
    Token* left_op;
    Expr* middle;
    Token* right_op;
    Expr* right;
};