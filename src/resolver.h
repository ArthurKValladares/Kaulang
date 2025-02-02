#pragma once


#include <vector>
#include <unordered_map>

#include "parser.h"

struct VariableStatus {
    bool defined = false;
    u64 uses = 0;
};
using ScopeMap = std::unordered_map<String, VariableStatus, StringHasher>;

enum class FunctionType {
    NONE,
    FUNCTION,
    INITIALIZER,
    METHOD,
};

enum class ClassType {
    NONE,
    CLASS,
};

struct KauCompiler;
struct Resolver {
    void resolve(KauCompiler* compiler, Stmt* stmts, u64 stmts_len);
private:
    void visit_expr_stmt(KauCompiler* compiler, Stmt* stmt);
    void visit_block_stmt(KauCompiler* compiler, Stmt* stmt);
    void visit_var_stmt(KauCompiler* compiler, Stmt* stmt);
    void visit_fn_stmt(KauCompiler* compiler, Stmt* stmt);
    void visit_class_stmt(KauCompiler* compiler, Stmt* stmt);
    void visit_if_stmt(KauCompiler* compiler, Stmt* stmt);
    void visit_print_stmt(KauCompiler* compiler, Stmt* stmt);
    void visit_return_stmt(KauCompiler* compiler, Stmt* stmt);
    void visit_while_stmt(KauCompiler* compiler, Stmt* stmt);
    
    void visit_variable_expr(KauCompiler* compiler, Expr* expr);
    void visit_assign_expr(KauCompiler* compiler, Expr* expr);
    void visit_binary_expr(KauCompiler* compiler, Expr* expr);
    void visit_fn_call_expr(KauCompiler* compiler, Expr* expr);
    void visit_grouping_expr(KauCompiler* compiler, Expr* expr);
    void visit_logical_expr(KauCompiler* compiler, Expr* expr);
    void visit_unary_expr(KauCompiler* compiler, Expr* expr);
    void visit_ternary_expr(KauCompiler* compiler, Expr* expr);
    void visit_get_expr(KauCompiler* compiler, Expr* expr);
    void visit_set_expr(KauCompiler* compiler, Expr* expr);
    void visit_this_expr(KauCompiler* compiler, Expr* expr);

    void declare(KauCompiler* compiler, Token* name);
    void define(Token* name);

    void begin_scope();
    void end_scope();

    void resolve_stmt(KauCompiler* compiler, Stmt* stmt);
    void resolve_expr(KauCompiler* compiler, Expr* expr);
    void resolve_fn(KauCompiler* compiler, Stmt* stmt, FunctionType ty);

    void resolve_local(KauCompiler* compiler, Expr* expr, const Token* token);

    void mark_resolved(KauCompiler* compiler, Expr* expr, int depth);

    // TOOD: using std structures for now
    std::vector<ScopeMap> scopes;

    FunctionType current_function = FunctionType::NONE;
    ClassType current_class = ClassType::NONE;
};