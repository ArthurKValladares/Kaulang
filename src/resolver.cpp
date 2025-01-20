#include "resolver.h"

void Resolver::resolve(Stmt* stmts, u64 stmts_len) {
    for (u64 i = 0; i < stmts_len; ++i) {
        resolve_stmt(stmts[i]);
    }
}

void Resolver::resolve_stmt(Stmt& stmt) {
    switch (stmt.ty) {
        case Stmt::Type::EXPR: {
            break;
        }
        case Stmt::Type::VAR_DECL: {
            declare(stmt.s_var_decl.name);
            if (stmt.s_var_decl.expr != nullptr) {
                resolve_expr(stmt.s_var_decl.expr);
            }
            define(stmt.s_var_decl.name);
            break;
        }
        case Stmt::Type::BLOCK: {
            begin_scope();
            resolve(stmt.s_block.stmts, stmt.s_block.size);
            end_scope();
            break;
        }
        case Stmt::Type::IF: {
            break;
        }
        case Stmt::Type::WHILE: {
            break;
        }
        case Stmt::Type::BREAK: {
            break;
        }
        case Stmt::Type::CONTINUE: {
            break;
        }
        case Stmt::Type::FN_DECLARATION: {
            break;
        }
        case Stmt::Type::RETURN: {
            break;
        }
        case Stmt::Type::PRINT: {
            break;
        }
        case Stmt::Type::ERR: {
            assert(false);
            break;
        }
    }
}

void Resolver::resolve_expr(Expr* expr) {
}

void Resolver::declare(Token* name) {
    if (scopes.empty()) {
        return;
    }

    ScopeMap& scope = scopes.back();
    scope.emplace(name->m_lexeme, false);
}

void Resolver::define(Token* name) {
    if (scopes.empty()) {
        return;
    }

    ScopeMap& scope = scopes.back();
    scope.emplace(name->m_lexeme, true);
}

void Resolver::begin_scope() {
    scopes.push_back(ScopeMap());
}

void Resolver::end_scope() {
    scopes.pop_back();
}