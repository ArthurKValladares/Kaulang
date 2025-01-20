#pragma once


#include <vector>
#include <unordered_map>

#include "parser.h"

using ScopeMap = std::unordered_map<String, bool, StringHasher>;

struct Resolver {
    void resolve(Stmt* stmts, u64 stmts_len);
private:
    void resolve_stmt(Stmt& stmt);
    void resolve_expr(Expr* expr);
    
    void declare(Token* name);
    void define(Token* name);

    void begin_scope();
    void end_scope();

    // TOOD: using std structures for now
    std::vector<ScopeMap> scopes;
};