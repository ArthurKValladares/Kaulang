#pragma once


#include <vector>
#include <unordered_map>

#include "parser.h"

using ScopeMap = std::unordered_map<String, bool, StringHasher>;

struct Resolver {
    void resolve(Stmt* stmts, u64 stmts_len);
private:
    void visit_block_stmt(Stmt* stmt);
    void visit_var_stmt(Stmt* stmt);
    void visit_fn_stmt(Stmt* stmt);
    void visit_variable_expr(Expr* expr);
    void visit_assign_expr(Expr* expr);

    void declare(Token* name);
    void define(Token* name);

    void begin_scope();
    void end_scope();

    void resolve_stmt(Stmt* stmt);
    void resolve_expr(Expr* expr);
    void resolve_fn(Stmt* stmt);

    void resolve_local(Expr* expr, const Token* token);

    void mark_resolved(Expr* expr, int depth);

    // TOOD: using std structures for now
    std::vector<ScopeMap> scopes;
};