#pragma once


#include <unordered_map>

#include "lib/map.h"
#include "lib/array.h"

#include "parser.h"

struct VariableStatus {
    bool defined = false;
    u64 uses = 0;
};

enum class FunctionType {
    NONE,
    FUNCTION,
    INITIALIZER,
    METHOD,
};

enum class ClassType {
    NONE,
    CLASS,
    SUBCLASS,
};

struct KauCompiler;
struct Resolver {
    void init(Arena *arena);

    void resolve(KauCompiler* compiler, Array<Stmt> stmts);
private:
    void visit_expr_stmt(KauCompiler* compiler, Stmt* stmt);
    void visit_block_stmt(KauCompiler* compiler, Stmt* stmt);
    void visit_var_stmt(KauCompiler* compiler, Stmt* stmt);
    void visit_fn_stmt(KauCompiler* compiler, Stmt* stmt);
    void visit_class_stmt(KauCompiler* compiler, Stmt* stmt);
    void visit_if_stmt(KauCompiler* compiler, Stmt* stmt);
    void visit_return_stmt(KauCompiler* compiler, Stmt* stmt);
    void visit_while_stmt(KauCompiler* compiler, Stmt* stmt);
    
    void visit_variable_expr(KauCompiler* compiler, Expr* expr);
    void visit_assign_expr(KauCompiler* compiler, Expr* expr);
    void visit_binary_expr(KauCompiler* compiler, Expr* expr);
    void visit_fn_call_expr(KauCompiler* compiler, Expr* expr);
    void visit_static_fn_call_expr(KauCompiler* compiler, Expr* expr);
    void visit_grouping_expr(KauCompiler* compiler, Expr* expr);
    void visit_logical_expr(KauCompiler* compiler, Expr* expr);
    void visit_unary_expr(KauCompiler* compiler, Expr* expr);
    void visit_ternary_expr(KauCompiler* compiler, Expr* expr);
    void visit_get_expr(KauCompiler* compiler, Expr* expr);
    void visit_set_expr(KauCompiler* compiler, Expr* expr);
    void visit_this_expr(KauCompiler* compiler, Expr* expr);
    void visit_super_expr(KauCompiler* compiler, Expr* expr);

    void declare(KauCompiler* compiler, Token* name);
    void define(Token* name);

    void begin_scope(KauCompiler* compiler);
    void end_scope();

    void resolve_stmt(KauCompiler* compiler, Stmt* stmt);
    void resolve_expr(KauCompiler* compiler, Expr* expr);
    void resolve_fn(KauCompiler* compiler, Stmt* stmt, FunctionType ty);

    void resolve_local(KauCompiler* compiler, Expr* expr, const Token* token);

    void mark_resolved(KauCompiler* compiler, Expr* expr, u64 depth);

    Array<Map> scopes;

    FunctionType current_function = FunctionType::NONE;
    ClassType current_class = ClassType::NONE;
};