#pragma once

#include "lib/string.h"
#include "lib/map.h"
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
struct LogicalBinaryExpr;
struct FnCallExpr;
struct GetExpr;
struct SetExpr;

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
    LogicalBinaryExpr* logical_binary;
    FnCallExpr* fn_call;
    GetExpr* get;
    SetExpr* set;
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
    static RuntimeError object_must_be_struct(const Token* token);
    static RuntimeError class_does_not_have_field(const Token* token);

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

struct Value;
struct Class {
    Class() {}
    Class(String name) 
        : m_name(name)

    {}

    bool contains(String field);
    void get(String field, Value& in_value);
    void set(String field, Value in_value);

    void print() const;

    StringMap fields;
    String m_name = String{};
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
        CLASS,
    };

    Type ty;
    union {
        bool b;
        int i;
        long l;
        float f;
        double d;
        String str = {};
        Class m_class;
    };

    void print() const;
};

struct ExprStmtPayload {
    Expr* expr;
};

struct VarDeclPayload {
    Token* name;
    Expr* initializer;
};

struct BlockPayload {
    Stmt* stmts;
    int size;
};

struct IfPayload {
    Expr* condition;
    Stmt* if_stmt;
    Stmt* else_stmt;
};

struct WhilePayload {
    Expr* condition;
    Stmt* body;
};

struct BreakContinuePayload {
    int line;
};

struct FnDeclarationPayload {
    Token* name;
    Token** params;
    u64 params_count;
    Stmt* body;
};

struct ClassDeclarationPayload {
    Token* name;
    Stmt* methods;
    u64 methods_count;
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
        CLASS_DECLARATION,
        RETURN,
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
        ClassDeclarationPayload s_class;
        ReturnPayload s_return;
    };

    Value evaluate(KauCompiler* compiler, Arena* arena, Environment* env, bool from_prompt, bool in_loop);
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
        GET,
        SET,
    };

    Type ty;
    ExprPayload expr;

    void print() const;
    RuntimeError evaluate(KauCompiler* compiler, Arena* arena, Environment* env, Value& in_value);
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

struct LogicalBinaryExpr {
    Expr* left;
    const Token* op;
    Expr* right;
};

struct FnCallExpr {
    Expr* callee;
    const Token* paren;
    Expr** arguments;
    u64 arguments_count;
};

struct GetExpr {
    Expr* class_expr;
    Token* member;
};

struct SetExpr {
    Expr* get;
    Expr* right;
};