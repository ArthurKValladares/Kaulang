#pragma once

#include "lib/string.h"

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
struct FnCallExpr;

struct Stmt;

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
    FnCallExpr* fn_call;
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
    static RuntimeError undeclared_function(const Token* token);
    static RuntimeError invalid_function_identifier(const Token* token);
    static RuntimeError invalid_function_argument(const Token* token);
    static RuntimeError wrong_number_arguments(const Token* token);

    bool is_ok() const;

    enum class Type {
        Ok,
        UNSUPPORTED_LITERAL,
        UNSUPPORTED_OPERATOR,
        WRONG_OPERANDS,
        DIVIDE_BY_ZERO,
        UNDEFINED_VARIABLE,
        UNDEFINED_FUNCTION,
        INVALID_IDENTIFIER,
        INVALID_ARGUMENT,
        WRONG_NUMBER_ARGUMENTS,
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
        LONG,
        STRING,
        BREAK,
        CONTINUE,
    };

    Type ty;
    union {
        bool b;
        int i;
        long l;
        float f;
        double d;
        String str = {};
    };

    void print() const;
};

struct ExprStmtPayload {
    Expr* expr;
};

struct VarDeclPayload {
    Token* name;
    Expr* expr;
};

struct BlockPayload {
    Stmt* stmts;
    int size;
};

struct IfPayload {
    Expr* expr;
    Stmt* if_stmt;
    Stmt* else_stmt;
};

struct WhilePayload {
    Expr* expr;
    Stmt* stmt;
};

struct BreakContinuePayload {
    int line;
};

struct FnDeclarationPayload {
    Token* name;
    Token** params;
    int params_size;
    Stmt* body;
};

// TODO: Review `Token` struct and add it to more payloads maybe if we need it for logging
struct ReturnPayload {
    Token* keyword;
    Expr* expr;
};

struct KauCompiler;
struct Environment;
struct Stmt {
    enum class Type {
        ERR,
        EXPR,
        VAR_DECL,
        BLOCK,
        IF,
        WHILE,
        BREAK,
        CONTINUE,
        FN_DECLARATION,
        RETURN,
        PRINT,
    };

    Type ty;
    union {
        ExprStmtPayload s_expr;
        VarDeclPayload s_var_decl;
        BlockPayload s_block;
        IfPayload s_if;
        WhilePayload s_while;
        BreakContinuePayload s_break_continue;
        FnDeclarationPayload fn_declaration;
        ReturnPayload s_return;
    };

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
        TERNARY,
        ASSIGNMENT,
        AND,
        OR,
        FN_CALL,
    };

    Type ty;
    ExprPayload expr;

    void print() const;
    RuntimeError evaluate(KauCompiler* compiler, Environment* env, Value& in_value);
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

struct FnCallExpr {
    Expr* callee;
    const Token* paren;
    std::vector<Expr*> arguments;
};