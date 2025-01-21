#include "resolver.h"

void Resolver::resolve(Stmt* stmts, u64 stmts_len) {
    for (u64 i = 0; i < stmts_len; ++i) {
        resolve_stmt(&stmts[i]);
    }
}

void Resolver::resolve_stmt(Stmt* stmt) {
    switch (stmt->ty) {
        case Stmt::Type::EXPR: {
            visit_expr_stmt(stmt);
            break;
        }
        case Stmt::Type::VAR_DECL: {
            visit_var_stmt(stmt);
            break;
        }
        case Stmt::Type::BLOCK: {
            visit_block_stmt(stmt);
            break;
        }
        case Stmt::Type::IF: {
            visit_if_stmt(stmt);
            break;
        }
        case Stmt::Type::WHILE: {
            visit_while_stmt(stmt);
            break;
        }
        case Stmt::Type::FN_DECLARATION: {
            visit_fn_stmt(stmt);
            break;
        }
        case Stmt::Type::RETURN: {
            visit_return_stmt(stmt);
            break;
        }
        case Stmt::Type::PRINT: {
            visit_print_stmt(stmt);
            break;
        }
        case Stmt::Type::BREAK: {
            break;
        }
        case Stmt::Type::CONTINUE: {
            break;
        }
        case Stmt::Type::ERR: {
            assert(false);
            break;
        }
    }
}

void Resolver::resolve_expr(Expr* expr) {
    switch (expr->ty) {
        case Expr::Type::LITERAL: {
            // TODO: Don't love that this is how I need to detect a variable, make it better
            if (expr->expr.literal->val->m_type == TokenType::IDENTIFIER) {
                visit_variable_expr(expr);
            }
            break;
        }
        case Expr::Type::UNARY: {
            break;
        }
        case Expr::Type::BINARY: {
            break;
        }
        case Expr::Type::GROUPING: {
            break;
        }
        case Expr::Type::TERNARY: {
            break;
        }
        case Expr::Type::ASSIGNMENT: {
            visit_assign_expr(expr);
            break;
        }
        case Expr::Type::AND: {
            break;
        }
        case Expr::Type::OR: {
            break;
        }
        case Expr::Type::FN_CALL: {
            break;
        }
        case Expr::Type::ERR: {
            assert(false);
            break;
        }
    }   
}

void Resolver::visit_expr_stmt(Stmt* stmt) {
    resolve_expr(stmt->s_expr.expr);
}

void Resolver::visit_block_stmt(Stmt* stmt) {
    begin_scope();
    resolve(stmt->s_block.stmts, stmt->s_block.size);
    end_scope();
}

void Resolver::visit_var_stmt(Stmt* stmt) {
    declare(stmt->s_var_decl.name);
    if (stmt->s_var_decl.initializer != nullptr) {
        resolve_expr(stmt->s_var_decl.initializer);
    }
    define(stmt->s_var_decl.name);
}

void Resolver::visit_fn_stmt(Stmt* stmt) {
    declare(stmt->fn_declaration.name);
    define(stmt->fn_declaration.name);

    resolve_fn(stmt);
}

void Resolver::visit_if_stmt(Stmt* stmt) {
    resolve_expr(stmt->s_if.condition);
    resolve_stmt(stmt->s_if.if_stmt);
    if (stmt->s_if.else_stmt->ty != Stmt::Type::ERR) {
        resolve_stmt(stmt->s_if.else_stmt);
    }
}

void Resolver::visit_print_stmt(Stmt* stmt) {
    // TODO: Print Stmt should be its own thing
    resolve_expr(stmt->s_expr.expr);
}

void Resolver::visit_return_stmt(Stmt* stmt) {
    resolve_expr(stmt->s_return.expr);
}

void Resolver::visit_while_stmt(Stmt* stmt) {
    resolve_expr(stmt->s_while.condition);
    resolve_stmt(stmt->s_while.body);
}

void Resolver::visit_variable_expr(Expr* expr) {
    const Token* token = expr->expr.literal->val;
    if (!scopes.empty() &&
        scopes.back().contains(token->m_lexeme) &&
        scopes.back().at(token->m_lexeme) == false) {
        std::println(stderr, "Can't read local varaible in its own initializer");
        exit(-1);
    }
    resolve_local(expr, token);
}

void Resolver::visit_assign_expr(Expr* expr) {
    resolve_expr(expr->expr.assignment->right);
    resolve_local(expr, expr->expr.literal->val);
}

void Resolver::resolve_local(Expr* expr, const Token* token) {
    for (i64 i = scopes.size() - 1; i >= 0; --i) {
        if (scopes[i].contains(token->m_lexeme)) {
            mark_resolved(expr, scopes.size() - 1 - i);
            return;
        }
    }
}

void Resolver::resolve_fn(Stmt* stmt) {
    begin_scope();
    for (u64 i = 0; i < stmt->fn_declaration.params_size; ++i) {
        Token* param = stmt->fn_declaration.params[i];
        declare(param);
        define(param);
    }
    resolve_stmt(stmt->fn_declaration.body);
    end_scope();
}

void Resolver::declare(Token* name) {
    if (scopes.empty()) {
        return;
    }

    scopes.back()[name->m_lexeme] = false;
}

void Resolver::define(Token* name) {
    if (scopes.empty()) {
        return;
    }

    scopes.back()[name->m_lexeme] = true;
}

void Resolver::begin_scope() {
    scopes.push_back(ScopeMap());
}

void Resolver::end_scope() {
    scopes.pop_back();
}

void Resolver::mark_resolved(Expr* expr, int depth) {
    // TODO
}