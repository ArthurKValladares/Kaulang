#pragma once

#include "tokens.h"

#include <string>
#include <vector>

struct Expr;
struct LiteralExpr;
struct GroupingExpr;
struct UnaryExpr;
struct BinaryExpr;
struct CommaExpr;
struct TernaryExpr;
struct PrintExpr;
struct AssignmentExpr;
struct AndExpr;
struct OrExpr;

union ExprPayload {
    LiteralExpr* literal;
    GroupingExpr* grouping;
    UnaryExpr* unary;
    BinaryExpr* binary;
    CommaExpr* comma;
    TernaryExpr* ternary;
    PrintExpr* print;
    AssignmentExpr* assignment;
    AndExpr* logical_and;
    OrExpr* logical_or;
};

struct RuntimeError {
    static RuntimeError ok();
    static RuntimeError unsupported_literal(const Token* token);
    static RuntimeError unsupported_binary_op(const Token* token);
    static RuntimeError unsupported_unary_op(const Token* token);
    static RuntimeError operands_must_be_equal(const Token* token);
    static RuntimeError operands_must_be_floats(const Token* token);
    static RuntimeError operands_must_be_strings(const Token* token);
    static RuntimeError operand_must_be_float(const Token* token);
    static RuntimeError operand_must_be_bool(const Token* token);
    static RuntimeError divide_by_zero(const Token* token);
    static RuntimeError operands_do_not_support_operator(const Token* token);
    static RuntimeError undeclared_variable(const Token* token);
    static RuntimeError undefined_variable(const Token* token);

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
    const Token* token;
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
        BREAK,
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
// for each type, make better later. Also these two foward-declares
// TODO: This is really really bad now, definitely need ot fix it.
struct KauCompiler;
struct Environment;
struct Stmt {
    enum class Type {
        ERR,
        PRINT,
        EXPR,
        VAR_DECL,
        BLOCK,
        IF,
        WHILE,
        BREAK,
    };

    Type ty;

    Token* name;
    Expr* expr;
    std::vector<Stmt> stmts;

    Value evaluate(KauCompiler* compiler, Environment* env, bool from_prompt, bool in_loop);
    void print();
};

struct Expr {
    enum class Type {
        ERR,
        LITERAL,
        UNARY,
        BINARY,
        GROUPING,
        COMMA,
        TERNARY,
        ASSIGNMENT,
        AND,
        OR,
    };

    Type ty;
    ExprPayload expr;

    void print() const;
    RuntimeError evaluate(Environment* env, Value& in_value);
};

// TODO: review these Token*'s later
struct LiteralExpr {
    const Token* val;
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
    const Token* op;
    Expr* right;
};

struct CommaExpr {
    Expr* left;
    const Token* op;
    Expr* right;
};

struct TernaryExpr {
    Expr* left;
    const Token* left_op;
    Expr* middle;
    const Token* right_op;
    Expr* right;
};

struct AssignmentExpr {
    const Token* id;
    Expr* right;
};

struct AndExpr {
    Expr* left;
    const Token* op;
    Expr* right;
};

struct OrExpr {
    Expr* left;
    const Token* op;
    Expr* right;
};